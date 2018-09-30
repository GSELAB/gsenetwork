#pragma once

#include <vector>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>

#include <core/Block.h>
#include <chain/BlockState.h>

using namespace core;

namespace chain {

typedef std::vector<BlockStatePtr> BranchType;

class RollbackState {
public:
    RollbackState();
    ~RollbackState();

    void close();

    BlockStatePtr getBlock(BlockID const& blockID) const;
    BlockStatePtr getBlock(uint64_t number) const;

    BlockStatePtr add(Block const& block, bool trust = false);
    BlockStatePtr add(BlockStatePtr nextBlock);

    void remove(BlockID const& blockID);
    void remove(uint64_t number);

    void add(HeaderConfirmation const& confirmation);

    BlockStatePtr const& head() const;

    // Given two head blocks, return two branchs of the fork graph that end with a common ancestor(same prior block)
    std::pair<BranchType, BranchType> fetchBranchFrom(BlockID const& first, BlockID const& second) const;

    // The invalid block would be removed,
    void setValidity(BlockStatePtr const& blockState, bool valid);
    void markInCurrentChain(BlockStatePtr const& blockState, bool inCurrentChain);
    void prune(BlockStatePtr const& blockState);

private:
    void setBFTIrreversible(BlockID blockID);

    // std::unique_ptr<>
    BlockStatePtr m_head;


};
} // namespace chain