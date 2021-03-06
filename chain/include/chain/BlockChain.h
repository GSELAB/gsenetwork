/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of GSENetwork.
 *
 * GSENetwork is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

#pragma once

#include <core/Guards.h>
#include <core/Types.h>
#include <core/Block.h>
#include <core/Producer.h>
#include <core/SubChain.h>
#include <core/Task.h>
#include <core/Transaction.h>
#include <core/Status.h>
#include <storage/Repository.h>
#include <net/NetController.h>
#include <crypto/GKey.h>
#include <core/Queue.h>
#include <chain/RollbackState.h>
#include <chain/Sync.h>
#include <listener/EventObserve.h>

using namespace net;
using namespace core;
using namespace listener;

namespace chain {

class BlockChain;

class Dispatch: public DispatchFace {
public:
    Dispatch(BlockChain* chain): m_chain(chain) {}

    virtual ~Dispatch() {}

    void processMsg(bi::tcp::endpoint const& from, BytesPacket const& msg) {}

    bool processMsg(bi::tcp::endpoint const& from, unsigned type, RLP const& rlp);

    bool processMsg(bi::tcp::endpoint const& from, unsigned type, bytes const& data) { return false; }

private:
    BlockChain *m_chain;
};

class BlockChainMessageFace {
public:
    virtual ~BlockChainMessageFace() {}

    virtual void broadcast(bi::tcp::endpoint const& from, Block& block) = 0;

    virtual void broadcast(bi::tcp::endpoint const& from, BlockPtr block) = 0;

    virtual void broadcast(bi::tcp::endpoint const& from, Transaction& tx) = 0;

    virtual void broadcast(bi::tcp::endpoint const& from, TransactionPtr tx) = 0;

    virtual void broadcast(bi::tcp::endpoint const& from, HeaderConfirmation& hc) = 0;

    virtual void broadcast(bi::tcp::endpoint const& from, HeaderConfirmationPtr hcp) = 0;

    virtual void send(HeaderConfirmation& bs) = 0;

    virtual void send(HeaderConfirmationPtr hcp) = 0;

    virtual void send(bi::tcp::endpoint const& to, Status& status) = 0;

    virtual void send(bi::tcp::endpoint const& to, StatusPtr status) = 0;

    virtual void send(bi::tcp::endpoint const& to, BlockState& bs) = 0;

    virtual void send(bi::tcp::endpoint const& to, BlockStatePtr bsp) = 0;

    virtual void schedule(ProducersConstRef producerList, int64_t timestamp) = 0;

    virtual Address getProducerAddress(unsigned idx) const = 0;

    virtual Producers getSortedProducerList() const = 0;

    virtual ProducerSnapshot getProducerSnapshot() const = 0;
};

enum BlockChainStatus {
    NormalStatus = 0x01,
    SyncStatus,
    ProducerStatus,
    Killed,
};

struct ByTxID;
struct ByTxTimestamp;
typedef boost::multi_index::multi_index_container<
    TransactionPtr,
    indexed_by<
        ordered_unique<tag<ByTxTimestamp>, const_mem_fun<Transaction, int64_t, &Transaction::getTimestamp>>,
        hashed_non_unique<tag<ByTxID>, const_mem_fun<Transaction, TxID, &Transaction::getHash>, std::hash<TxID>>
    >
> TxCacheMultiIndexType;

typedef boost::multi_index::multi_index_container<
    BlockPtr,
    indexed_by<
        ordered_unique<tag<ByUpBlockNumber>, const_mem_fun<Block, uint64_t, &Block::getNumber>, std::less<uint64_t>>,
        hashed_non_unique<tag<ByBlockID>, const_mem_fun<Block, BlockID, &Block::getHash>, std::hash<BlockID>>
    >
> BlockCacheMultiIndexType;

class BlockChain: public Task {
    friend class Sync;
public:
    struct MemoryItem {
        MemoryItem(): m_isDone(false) {}

        MemoryItem(uint64_t number, std::shared_ptr<runtime::storage::Repository> repository): m_isDone(false), m_blockNumber(number), m_repository(repository) {}

        ~MemoryItem() { m_repository.reset(); }

        std::shared_ptr<runtime::storage::Repository> getRepository() const { return m_repository; }

        void setRepository(std::shared_ptr<runtime::storage::Repository> repository) { m_repository = repository; }

        uint64_t getBlockNumber() const { return m_blockNumber; }

        BlockID getBlockID() const { return m_repository->getBlock().getHash(); }

        Block getBlock() const { return m_repository->getBlock(); }

        void setBlockNumber(uint64_t blockNumber) { m_blockNumber = blockNumber; }

        void bonus(Address const& address, uint64_t value) { if (m_repository) m_repository->bonus(address, value); }

        bool isDone() const { return m_isDone; }

        void setDone() { m_isDone = true; }

        void setParentEmpty() { m_repository->setParentNULL(); }

        void commit() { m_repository->commit(); }

        bool m_isDone = false;
        uint64_t m_blockNumber;
        std::shared_ptr<runtime::storage::Repository> m_repository;
    };

    typedef core::Queue<std::shared_ptr<MemoryItem>> Queue_t;

public:
    BlockChain(crypto::GKey const& key, DatabaseController* dbc, BlockChainMessageFace *messageFace, ChainID chainID = GSE_UNKNOWN_NETWORK);

    virtual ~BlockChain();

    void init();

    void start();

    void stop();

    void pushSchedule();

    void initializeRollbackState();

    DatabaseController* getDBC() const { return m_dbc; }

    ChainID const& getChainID() const { return m_chainID; }

    BlockChainStatus getBlockChainStatus() const { return m_blockChainStatus; }

    void setBlockChainStatus(BlockChainStatus status) { m_blockChainStatus = status; }

    void setChainID(ChainID const& chainID) { m_chainID = chainID; }

    bool processBlock(BlockPtr block);

    bool processProducerBlock(BlockPtr block);

    void processTransaction(BlockPtr block, Transaction const& transaction, MemoryItem* mItem);

    bool checkBifurcation(BlockPtr block);

    DispatchFace* getDispatcher() const { return m_dispatcher; }

    uint64_t getLastBlockNumber() const;

    Block getLastBlock() const;

    Block getBlockByNumber(uint64_t number);

    BlockState getBlockStateByNumber(uint64_t number);

    std::shared_ptr<Transaction> getTransactionFromCache();

    BlockPtr getBlockFromCache();

    void onSolidifiable(BlockStatePtr bsp);

    Address getExpectedProducer(int64_t timestamp) const;

    Producer getProducer(Address const& address);

    Producers getProducerListFromRepo() const;

    void schedule(int64_t timestamp);

    void updateActiveProducers(BlockPtr block);

    void registerObserver(Observer<Object*> const& observer) { m_observe.add(observer); }

    bool checkProducer(BlockPtr block) const;

    bool checkDuplicateTx(Block const& block) const;

    void updateProducerRecord();

public: /// used by rpc
    bool addRPCTx(Transaction& tx);

    Transaction getTx(TxID const& txID);

    Account getAccount(Address const& address);

    uint64_t getBalance(Address const& address);

    uint64_t getHeight() const;

    uint64_t getSolidifyHeight() const;


public: /// Used by network
    void preProcessTx(bi::tcp::endpoint const& from, Transaction& tx) { preProcessTx(tx); }

    void preProcessBlock(bi::tcp::endpoint const& from, Block& block);

    void processTxMessage(bi::tcp::endpoint const& from, Transaction& tx);

    void processBlockMessage(bi::tcp::endpoint const& from, Block& block);

    void processSyncBlockMessage(bi::tcp::endpoint const& from, Block& block);

    void processConfirmationMessage(bi::tcp::endpoint const& from, HeaderConfirmation& confirmation);

    void processStatusMessage(bi::tcp::endpoint const& from, Status& status);

    void processBlockStateMessage(bi::tcp::endpoint const& from, BlockState const& bs);

    bool isExist(Transaction& tx);

    bool isExistInRepo(Transaction& tx);

    bool isExist(Block& block);

protected:
    std::shared_ptr<MemoryItem> addMemoryItem(BlockPtr block);

    void cancelMemoryItem();

    void popBlockState();

    void doProcessBlock(BlockPtr block);

    void eraseSolicitedTx(BlockPtr block);

    void preProcessTx(Transaction& tx);

private:
    virtual void doWork() override;

private:
    DatabaseController* m_dbc = nullptr;

    BlockChainMessageFace *m_messageFace = nullptr;

    GKey m_key;
    ChainID m_chainID = GSE_UNKNOWN_NETWORK;

    DispatchFace* m_dispatcher;

    Sync* m_sync = nullptr;

    mutable Mutex x_memoryQueue;
    Queue_t m_memoryQueue;

    RollbackState m_rollbackState;
    BlockStatePtr m_head;

    uint64_t m_solidifyIndex;
    BlockChainStatus m_blockChainStatus = ProducerStatus;

    mutable Mutex x_historyTxCache;
    FixedQueue<TxID, 4096> m_historyTxCache;

    mutable Mutex x_historyBroadcastBlockCache;
    FixedQueue<BlockID, 8> m_historyBroadcastBlockCache;

    mutable Mutex x_txCache;
    TxCacheMultiIndexType m_txCache;

    mutable Mutex x_blockCache;
    BlockCacheMultiIndexType m_blockCache;

    ActiveProducerSnapshot m_currentActiveProducers;

    ProducerSnapshot m_prevPS;
    ProducerSnapshot m_currentPS;

    EventObserve<Object*> m_observe;

    mutable Mutex x_latestBlock;
    uint64_t m_latestBlockNumber;
    Block m_latestBlock;

    std::map<Address, int64_t> m_producerRecord;

    int64_t m_scheduleUpdateBlockTimestamp = 0;
};
} // end namespace
