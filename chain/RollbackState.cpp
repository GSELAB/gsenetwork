#include <chain/RollbackState.h>
#include <core/Log.h>

using namespace core;

namespace chain {

RollbackState::RollbackState()
{

}

RollbackState::~RollbackState()
{
    close();
}

void RollbackState::close()
{
    if (m_index.size() == 0) return;
    m_index.clear();
}

BlockStatePtr RollbackState::getBlock(BlockID const& blockID) const
{
    auto itr = m_index.find(blockID);
    if (itr != m_index.end()) return *itr;
    return EmptyBlockStatePtr;
}

BlockStatePtr RollbackState::getBlock(uint64_t number) const
{
    auto const& numberIdx = m_index.get<ByBlockNumber>();
    auto itr = numberIdx.find(number);
    if (itr != numberIdx.end())
        return *itr;
    return EmptyBlockStatePtr;
}

void RollbackState::set(BlockStatePtr bsp)
{
    auto ret = m_index.insert(bsp);
    if (!ret.second) {
        throw RollbackStateException("Duplicate state!");
    }

    if (!m_head) {
        m_head = bsp;
    } else if (m_head->m_blockNumber < bsp->m_blockNumber) {
        m_head = bsp;
    }
}

BlockStatePtr RollbackState::add(Block& block, ProducerSnapshot const& ps, bool trust)
{
    if (!m_head) {
        throw RollbackStateException("Not set head block!");
    }

    auto& item = m_index.get<ByBlockID>();
    auto existing = item.find(block.getHash());
    if (existing != item.end()) {
        throw RollbackStateException("Block has exist!");
    }

    auto parent = item.find(block.getBlockHeader().getParentHash());
    if (parent == item.end()) {
        CERROR << "Unlink block, id:" << block.getBlockHeader().getParentHash();
        throw RollbackStateException("Unlink block, id!");
    }

    auto bsp = std::make_shared<BlockState>(block);
    if (!bsp) {
        throw RollbackStateException("Make shared block state failed!");
    }

    bsp->setProducerSnapshot(ps);
    return add(bsp);
}

BlockStatePtr RollbackState::add(BlockStatePtr nextBSP)
{
    auto ret = m_index.insert(nextBSP);
    if (!ret.second) {
        throw RollbackStateException("Duplicate block state add!");
    }

    m_head = *m_index.get<ByBlockNumber>().begin();
    auto _head = *m_index.get<ByMultiBlockNumber>().begin();
    uint64_t mutilNumber = _head->m_bftSolidifyBlockNumber;
    BlockStatePtr oldest = *m_index.get<ByUpBlockNumber>().begin();
    CINFO << "Old number:" << oldest->m_blockNumber << "\tirreversible number:" << mutilNumber;
    if (oldest->m_blockNumber < mutilNumber) {
        auto solidifyBSP = getBlock(mutilNumber);
        prune(solidifyBSP);
    }
    return nextBSP;
}

void RollbackState::remove(BlockID const& blockID)
{
    auto itr = m_index.find(blockID);
    if (itr == m_index.end())
        return;

    auto prevBlockID = (*itr)->getPrev();
    auto prevItr = m_index.find(prevBlockID);
    if (prevItr != m_index.end())
        remove(prevBlockID);

    m_index.erase(itr);
}

void RollbackState::remove(uint64_t number)
{
    auto& item = m_index.get<ByBlockNumber>();
    auto itr = item.find(number);
    if (itr == item.end()) return;
    remove((*itr)->m_blockID);
}

void RollbackState::add(HeaderConfirmation const& confirmation)
{
    BlockStatePtr bsp = getBlock(confirmation.getBlockID());
    if (!bsp) {
        // throw RollbackStateException("Confirmation block id not found!");
        return;
    }

    bsp->addConfirmation(confirmation);
    CINFO << "Add confirmation - number:" << confirmation.getNumber() << " size = " << bsp->getConfirmationsSize() << " active-p(" << bsp->m_activeProucers.size() << ")";
    if (bsp->m_bftSolidifyBlockNumber < bsp->m_blockNumber && bsp->getConfirmationsSize() >= ((bsp->m_activeProucers.size() * 2) / 3)) {
        setBFTSolidify(bsp->m_blockID);
    }
}

void RollbackState::setSolidifyNumber(uint64_t number)
{
    m_solidifyNumber = number;
}

void RollbackState::addSyncBlockState(BlockState const& bs)
{
    if (bs.m_blockNumber <= m_solidifyNumber)
        return;

    for (auto i : bs.getConfirmations())
        add(i);
}

BlockStatePtr const& RollbackState::head() const
{
    return m_head;
}

// Given two head blocks, return two branches of the fork graph that end with a common ancestor(same prior block)
std::pair<BranchType, BranchType> RollbackState::fetchBranchFrom(BlockID const& first, BlockID const& second) const
{
    std::pair<BranchType, BranchType> ret;
    BlockStatePtr firstItem = getBlock(first);
    BlockStatePtr secondItem = getBlock(second);
    while (firstItem->m_blockNumber > secondItem->m_blockNumber) {
        ret.first.push_back(firstItem);
        firstItem = getBlock(firstItem->getPrev());
        if (!firstItem) {
            CERROR << "Block ID:" << firstItem->getPrev() << " not exist!";
            throw RollbackStateException("Block ID not exist!");
        }
    }

    while (secondItem->m_blockNumber > firstItem->m_blockNumber) {
        ret.second.push_back(secondItem);
        secondItem = getBlock(secondItem->getPrev());
        if (!secondItem) {
            CERROR << "Block ID:" << secondItem->getPrev() << " not exist!";
            throw RollbackStateException("Block ID not exist!");
        }
    }

    while (firstItem->getPrev() != secondItem->getPrev()) {
        ret.first.push_back(firstItem);
        ret.second.push_back(secondItem);
        firstItem = getBlock(firstItem->getPrev());
        secondItem = getBlock(secondItem->getPrev());
        if (!firstItem || !secondItem) {
            CERROR << "First or second block not exist!";
            RollbackStateException("First or second block not exist!");
        }
    }

    if (firstItem && secondItem) {
        ret.first.push_back(firstItem);
        ret.second.push_back(secondItem);
    }

    return ret;
}

void RollbackState::setValidity(BlockStatePtr const& blockState, bool valid)
{
    if (valid) {
        blockState->m_validated = true;
    } else {
        remove(blockState->m_blockID);
    }
}

void RollbackState::markInCurrentChain(BlockStatePtr const& blockState, bool inCurrentChain)
{
    if (blockState->m_inCurrentChain == inCurrentChain) return;
    auto& blockIdx = m_index.get<ByBlockID>();
    auto itr = blockIdx.find(blockState->m_blockID);
    if (itr == blockIdx.end()) {
        throw RollbackStateException("Mark unexist block in chain!");
    }

    blockIdx.modify(itr, [&](auto& bsp) {
        bsp->m_inCurrentChain = inCurrentChain;
    });
}

void RollbackState::prune(BlockStatePtr const& bsp)
{
    auto _bsp = m_index.find(bsp->m_blockID);
    if (_bsp != m_index.end()) {
        m_irreversible(*_bsp);
    }
}

void RollbackState::setBFTSolidify(BlockID blockID)
{
    auto& idx = m_index.get<ByBlockID>();
    auto itr = idx.find(blockID);
    uint64_t number = (*itr)->m_blockNumber;
    idx.modify(itr, [&](auto& bsp) {
        bsp->m_bftSolidifyBlockNumber = bsp->m_blockNumber;
    });
}
}
