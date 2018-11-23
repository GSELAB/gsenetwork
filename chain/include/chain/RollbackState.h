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
using namespace boost::multi_index;

namespace chain {

typedef std::vector<BlockStatePtr> BranchType;

struct ByBlockID;
struct ByBlockNumber;
struct ByUpBlockNumber;
struct ByPrev;
struct ByMultiBlockNumber;
typedef boost::multi_index::multi_index_container<
    BlockStatePtr,
    indexed_by<
        hashed_unique< tag<ByBlockID>, member<BlockState, BlockID, &BlockState::m_blockID>, std::hash<BlockID> >,
        ordered_non_unique< tag<ByPrev>, const_mem_fun<BlockState, BlockID, &BlockState::getPrev> >,
        ordered_non_unique< tag<ByBlockNumber>, member<BlockState, uint64_t, &BlockState::m_blockNumber>, std::greater<uint64_t> >,
        ordered_non_unique< tag<ByUpBlockNumber>, member<BlockState, uint64_t, &BlockState::m_blockNumber>, std::less<uint64_t> >,
        ordered_non_unique< tag<ByMultiBlockNumber>,
            composite_key<BlockState,
                member<BlockState, uint64_t,&BlockState::m_bftSolidifyBlockNumber>,
                member<BlockState, uint64_t,&BlockState::m_blockNumber>
            >,
            composite_key_compare< std::greater<uint64_t>, std::less<uint64_t> >
        >
    >
> RollbackMultiIndexType;

typedef boost::multi_index::multi_index_container<
    BlockState,
    indexed_by<
        hashed_unique<tag<ByBlockID>, member<BlockState, BlockID, &BlockState::m_blockID>, std::hash<BlockID>>,
        ordered_non_unique<tag<ByUpBlockNumber>, member<BlockState, uint64_t, &BlockState::m_blockNumber>, std::greater<uint64_t>>
    >
> BlockStateCacheIndexType;

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

    void addSyncBlockState(BlockState const& bs);

    BlockStatePtr const& head() const { return m_head; }

    std::pair<BranchType, BranchType> fetchBranchFrom(BlockID const& first, BlockID const& second) const;

    void solidifiable(BlockStatePtr const& bsp);

    void setBFTSolidify(BlockID blockID);

    void setSolidifyNumber(uint64_t number);

    uint64_t getSolidifyNumber() const { return m_solidifyNumber; }

public:
    boost::signals2::signal<void(BlockStatePtr)> m_solidifiable;

private:
    BlockStatePtr m_head;

    mutable Mutex x_index;
    RollbackMultiIndexType m_index;

    uint64_t m_solidifyNumber = 0;
};

}