#include <chain/RollbackState.h>
#include <core/Log.h>

using namespace core;

namespace chain {

RollbackState::RollbackState()
{

}

RollbackState::~RollbackState()
{

}

void RollbackState::close()
{

}

BlockStatePtr RollbackState::getBlock(BlockID const& blockID) const
{
    return BlockStatePtr();
}

BlockStatePtr RollbackState::getBlock(uint64_t number) const
{
    return BlockStatePtr();
}

void RollbackState::set(BlockStatePtr bsp)
{

}

BlockStatePtr RollbackState::add(Block const& block, bool trust)
{
    //if (!block) {
    //    CERROR << "Add null block";
    //}

    if (!m_head) {
        CERROR << "No head block";
    }

    return BlockStatePtr();
}

BlockStatePtr RollbackState::add(BlockStatePtr nextBlock)
{
    return BlockStatePtr();
}

void RollbackState::remove(BlockID const& blockID)
{

}

void RollbackState::remove(uint64_t number)
{

}

void RollbackState::add(HeaderConfirmation const& confirmation)
{

}

BlockStatePtr const& RollbackState::head() const
{

}

// Given two head blocks, return two branchs of the fork graph that end with a common ancestor(same prior block)
std::pair<BranchType, BranchType> RollbackState::fetchBranchFrom(BlockID const& first, BlockID const& second) const
{

}

// The invalid block would be removed,
void RollbackState::setValidity(BlockStatePtr const& blockState, bool valid)
{

}

void RollbackState::markInCurrentChain(BlockStatePtr const& blockState, bool inCurrentChain)
{

}

void RollbackState::prune(BlockStatePtr const& blockState)
{

}

void RollbackState::setBFTIrreversible(BlockID blockID)
{

}
}