#include <chain/RollbackState.h>
#include <core/CommonIO.h>
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
    uint64_t solidifyNumber = _head->m_bftSolidifyBlockNumber;
    BlockStatePtr oldest = *m_index.get<ByUpBlockNumber>().begin();
    if (nextBSP->m_block.isSyncBlock()) {
        CWARN << "Process sync block(number:" << nextBSP->m_blockNumber
              << " tx-size:"
              << nextBSP->m_block.getTransactionsSize()
              << ") to rollback state - current solidify number:"
              << solidifyNumber;
    } else {
        CWARN << "Process broadcast block(number:" << nextBSP->m_blockNumber
              << " tx-size:"
              << nextBSP->m_block.getTransactionsSize()
              << ") to rollback state - current solidify number:"
              << solidifyNumber;
    }
    if (oldest->m_blockNumber < solidifyNumber) {
        auto solidifyBSP = getBlock(solidifyNumber);
        solidifiable(solidifyBSP);
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
    if (!bsp)
        return;

    bsp->addConfirmation(confirmation);
    if (bsp->m_bftSolidifyBlockNumber < bsp->m_blockNumber && bsp->getConfirmationsSize() >= ((bsp->m_activeProucers.size() * 2) / 3)) {
        /// CWARN << "Confirmed number:" << confirmation.getNumber()
        ///      << "\tsize:" << bsp->getConfirmationsSize()
        ///      << "\tactive-p:" << bsp->m_activeProucers.size();
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

// Given two head blocks, return two branches of the fork graph that end with a common ancestor(same prior block)
std::pair<BranchType, BranchType> RollbackState::fetchBranchFrom(BlockID const& first, BlockID const& second) const
{
    std::pair<BranchType, BranchType> result;
    BlockStatePtr firstItem = getBlock(first);
    BlockStatePtr secondItem = getBlock(second);
    while (firstItem->m_blockNumber > secondItem->m_blockNumber) {
        result.first.push_back(firstItem);
        BlockID prevBlockID = firstItem->getPrev();
        firstItem = getBlock(prevBlockID);
        if (!firstItem) {
            CWARN << "First block id(" << prevBlockID << ") not exist in rollback state";
            throw RollbackStateAncestorException("First block id(" + toString(prevBlockID) + ") not exist in rollback state");
        }

    }

    while (secondItem->m_blockNumber > firstItem->m_blockNumber) {
        result.second.push_back(secondItem);
        BlockID prevBlockID = secondItem->getPrev();
        secondItem = getBlock(prevBlockID);
        if (!secondItem) {
            CWARN << "Second block id(" << prevBlockID << ") not exist in rollback state";
            throw RollbackStateAncestorException("Second block id(" + toString(prevBlockID) + ") not exist in rollback state");
        }
    }

    while (firstItem->getPrev() != secondItem->getPrev()) {
        result.first.push_back(firstItem);
        result.second.push_back(secondItem);
        firstItem = getBlock(firstItem->getPrev());
        secondItem = getBlock(secondItem->getPrev());
        if (!firstItem || !secondItem) {
            CWARN << "The same ancestor block not exist";
            RollbackStateAncestorException("The same ancestor block not exist");
        }
    }

    return result;
}

void RollbackState::solidifiable(BlockStatePtr const& bsp)
{
    auto _bsp = m_index.find(bsp->m_blockID);
    if (_bsp != m_index.end()) {
        m_solidifiable(*_bsp);
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
