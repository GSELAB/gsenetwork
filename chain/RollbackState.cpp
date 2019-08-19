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
    Guard l{x_index};
    if (m_index.size()) {
        m_index.clear();
    }
}

BlockStatePtr RollbackState::getBlock(BlockID const& blockID) const
{
    Guard l{x_index};
    auto itr = m_index.find(blockID);
    if (itr != m_index.end()) {
        return *itr;
    }

    return EmptyBlockStatePtr;
}

BlockStatePtr RollbackState::getBlock(uint64_t number) const
{
    Guard l{x_index};
    auto const& numberIdx = m_index.get<ByBlockNumber>();
    auto itr = numberIdx.find(number);
    if (itr != numberIdx.end()) {
        return *itr;
    }

    return EmptyBlockStatePtr;
}

void RollbackState::set(BlockStatePtr bsp)
{
    {
        Guard l{x_index};
        auto ret = m_index.insert(bsp);
        if (!ret.second) {
            throw RollbackStateException("Duplicate state!");
        }
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

    {
        Guard l{x_index};
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
    uint64_t solidifyNumber;
    BlockStatePtr oldest;
    {
        Guard l{x_index};
        auto ret = m_index.insert(nextBSP);
        if (!ret.second) {
            throw RollbackStateException("Duplicate block state added!");
        }

        m_head = *m_index.get<ByBlockNumber>().begin();
        auto _head = *m_index.get<ByMultiBlockNumber>().begin();
        if (!m_head || !_head) {
            CWARN << "Add block state failed due to bad header or multi header";
            throw RollbackStateException("Add block state failed due to bad header or multi header");
        }

        solidifyNumber = _head->m_bftSolidifyBlockNumber;
        oldest = *m_index.get<ByUpBlockNumber>().begin();
    }

    if (nextBSP->m_block.isSyncBlock()) {
        CWARN << "Received [s] block:" << nextBSP->m_blockNumber
              << "\ttxns:" << std::setw(5) << std::left << nextBSP->m_block.getTransactionsSize()
              << "\tsolidify:" << m_solidifyNumber;
    } else {
        CWARN << "Received [b] block:" << nextBSP->m_blockNumber
              << "\ttxns:" << std::setw(5) << std::left << nextBSP->m_block.getTransactionsSize()
              << "\tsolidify:" << m_solidifyNumber;
    }

    if (oldest->m_blockNumber < solidifyNumber) {
        auto solidifyBSP = getBlock(solidifyNumber);
        if (!solidifyBSP) {
            CWARN << "Error occurred while trying to solidify block:"
                  << solidifyNumber
                  << ", the block state could not be found.";
            return nextBSP;
        }
        m_solidifiable(solidifyBSP);
    }

    return nextBSP;
}

void RollbackState::remove(BlockID const& blockID)
{
    auto itr = m_index.find(blockID);
    if (itr == m_index.end()) {
        return;
    }

    auto prevBlockID = (*itr)->getPrev();
    auto prevItr = m_index.find(prevBlockID);
    if (prevItr != m_index.end()) {
        remove(prevBlockID);
    }

    {
        Guard l{x_index};
        m_index.erase(itr);
    }
}

void RollbackState::remove(uint64_t number)
{
    bool needDel = false;
    BlockID blockID;
    {
        Guard l{x_index};
        auto& item = m_index.get<ByBlockNumber>();
        auto itr = item.find(number);
        if (itr != item.end()) {
            blockID = (*itr)->m_blockID;
            needDel = true;
        }
    }

    if (needDel) {
        remove(blockID);
    }
}

void RollbackState::add(HeaderConfirmation const& confirmation)
{
    BlockStatePtr bsp = getBlock(confirmation.getBlockID());
    if (!bsp) {
        return;
    }

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
    while (firstItem && secondItem && firstItem->m_blockNumber > secondItem->m_blockNumber) {
        result.first.push_back(firstItem);
        BlockID prevBlockID = firstItem->getPrev();
        firstItem = getBlock(prevBlockID);
        if (!firstItem) {
            CWARN << "First block id(" << prevBlockID << ") not exist in rollback state";
            throw RollbackStateAncestorException("First block id(" + toString(prevBlockID) + ") not exist in rollback state");
        }

    }

    while (secondItem && firstItem && secondItem->m_blockNumber > firstItem->m_blockNumber) {
        result.second.push_back(secondItem);
        BlockID prevBlockID = secondItem->getPrev();
        secondItem = getBlock(prevBlockID);
        if (!secondItem) {
            CWARN << "Second block id(" << prevBlockID << ") not exist in rollback state";
            throw RollbackStateAncestorException("Second block id(" + toString(prevBlockID) + ") not exist in rollback state");
        }
    }

    while (firstItem && secondItem && firstItem->getPrev() != secondItem->getPrev()) {
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

bool RollbackState::rollbackTo(BlockStatePtr first, BlockStatePtr second, BranchType& branch) const
{
    BlockStatePtr prevItem = second;
    while (first && prevItem && first->m_blockNumber < prevItem->m_blockNumber) {
        branch.push_back(prevItem);
        BlockID id = prevItem->getPrev();
        prevItem = getBlock(id);
        if (!prevItem) {
            CWARN << "Block id(" << id << ") not exist in rollback state";
            branch.clear();
            return false;
        }
    }

    if (first->m_blockNumber != prevItem->m_blockNumber) {
        CWARN << "Not find the same number(" << first->m_blockNumber << ")";
        branch.clear();
        return false;
    }

    if (first->getPrev() != prevItem->getPrev()) {
        CWARN << "Not find the same prev Block ID(" << first->getPrev() << ")";
        branch.clear();
        return false;
    }

    return true;
}

void RollbackState::setBFTSolidify(BlockID blockID)
{
    Guard l{x_index};
    auto& idx = m_index.get<ByBlockID>();
    auto itr = idx.find(blockID);
    if (itr == idx.end()) {
        return;
    }

    uint64_t number = (*itr)->m_blockNumber;
    idx.modify(itr, [&](auto& bsp) {
        bsp->m_bftSolidifyBlockNumber = bsp->m_blockNumber;
    });
}
}
