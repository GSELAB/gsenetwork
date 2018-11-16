#pragma once

#include <vector>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/signals2/signal.hpp>

#include <core/Block.h>
#include <chain/BlockState.h>

using namespace core;

namespace chain {

using namespace boost::multi_index;

typedef std::vector<BlockStatePtr> BranchType;

struct ByBlockID;
struct ByBlockNumber;
struct ByUpBlockNumber;
struct ByPrev;
struct ByMultiBlockNumber;

/*
typedef boost::multi_index::multi_index_container<
    BlockStatePtr,
    indexed_by<
        hashed_unique<tag<ByBlockID>, member<BlockState, BlockID, &BlockState::m_blockID>, std::hash<BlockID>>,
        ordered_non_unique<tag<ByPrev>, const_mem_fun<BlockState, BlockID const&, &BlockState::getPrev>>,
        ordered_non_unique<tag<ByBlockNumber>, member<BlockState, uint64_t, &BlockState::m_blockNumber>>,
        ordered_non_unique<tag<ByMultiBlockNumber>,
            composite_key<BlockState,
                // member<BlockState, uint64_t,&BlockState::m_dposIrreversibleBlockNumber>,
                member<BlockState, uint64_t,&BlockState::m_bftIrreversibleBlockNumber>,
                member<BlockState, uint64_t,&BlockState::m_blockNumber>
            >,
            composite_key_compare<
                // std::greater<uint64_t>,
                std::greater<uint64_t>,
                std::greater<uint64_t>>
        >
    >
> RollbackMultiIndexType;
*/

typedef boost::multi_index::multi_index_container<
    BlockStatePtr,
    indexed_by<
        hashed_unique<tag<ByBlockID>, member<BlockState, BlockID, &BlockState::m_blockID>, std::hash<BlockID>>,
        ordered_non_unique<tag<ByPrev>, const_mem_fun<BlockState, BlockID const&, &BlockState::getPrev>>,
        ordered_non_unique<tag<ByBlockNumber>, member<BlockState, uint64_t, &BlockState::m_blockNumber>, std::greater<uint64_t>>,
        ordered_non_unique<tag<ByUpBlockNumber>, member<BlockState, uint64_t, &BlockState::m_blockNumber>, std::less<uint64_t>>,
        ordered_non_unique<tag<ByMultiBlockNumber>,
            composite_key<BlockState,
                member<BlockState, uint64_t,&BlockState::m_bftIrreversibleBlockNumber>,
                member<BlockState, uint64_t,&BlockState::m_blockNumber>
            >,
            composite_key_compare<
                std::greater<uint64_t>,
                std::greater<uint64_t>>
        >
    >
> RollbackMultiIndexType;

class RollbackState {
public:
    RollbackState();
    ~RollbackState();

    void close();

    BlockStatePtr getBlock(BlockID const& blockID) const;
    BlockStatePtr getBlock(uint64_t number) const;

    void set(BlockStatePtr bsp);

    BlockStatePtr add(Block& block, ProducerSnapshot const& ps, bool trust = false);
    BlockStatePtr add(BlockStatePtr nextBSP);

    void remove(BlockID const& blockID);
    void remove(uint64_t number);

    void add(HeaderConfirmation const& confirmation);

    BlockStatePtr const& head() const;

    // Given two head blocks, return two branchs of the fork graph that end with a common ancestor(same prior block)
    std::pair<BranchType, BranchType> fetchBranchFrom(BlockID const& first, BlockID const& second) const;

    // The invalid block would be removed,
    void setValidity(BlockStatePtr const& bsp, bool valid);
    void markInCurrentChain(BlockStatePtr const& bsp, bool inCurrentChain);
    void prune(BlockStatePtr const& bsp);

public:
    boost::signals2::signal<void(BlockStatePtr)> m_irreversible;

private:
    void setBFTIrreversible(BlockID blockID);

    // std::unique_ptr<>
    BlockStatePtr m_head;
    RollbackMultiIndexType m_index;


};
} // namespace chain