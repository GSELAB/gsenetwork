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

using namespace net;
using namespace core;

namespace chain {

class BlockChain;

class Dispatch: public DispatchFace {
public:
    Dispatch(BlockChain* chain): m_chain(chain) {}

    virtual ~Dispatch() {}

    void processMsg(bi::tcp::endpoint const& from, BytesPacket const& msg);

    bool processMsg(bi::tcp::endpoint const& from, unsigned type, RLP const& rlp);

    bool processMsg(bi::tcp::endpoint const& from, unsigned type, bytes const& data);

    static std::unique_ptr<core::Object> interpretObject(bi::tcp::endpoint const& from, BytesPacket const& msg);

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

    virtual void schedule(ProducersConstRef producerList) = 0;

    virtual Address getProducerAddress(unsigned idx) const = 0;
};

enum BlockChainStatus {
    NormalStatus = 0x01,
    SyncStatus,
    ProducerStatus,
};

struct ByTxID;
struct ByTxTimestamp;
typedef boost::multi_index::multi_index_container<
    TransactionPtr,
    indexed_by<
        ordered_unique<tag<ByTxTimestamp>, const_mem_fun<Transaction, int64_t, &Transaction::getTimestamp>>,
        hashed_non_unique<tag<ByTxID>, mem_fun<Transaction, TxID const&, &Transaction::getHash>, std::hash<TxID>>
    >
> TxCacheMultiIndexType;

typedef boost::multi_index::multi_index_container<
    BlockPtr,
    indexed_by<
        ordered_unique<tag<ByUpBlockNumber>, const_mem_fun<Block, uint64_t, &Block::getNumber>, std::less<uint64_t>>,
        hashed_non_unique<tag<ByBlockID>, mem_fun<Block, BlockID const&, &Block::getHash>, std::hash<BlockID>>
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

        BlockID const& getBlockID() const { return m_repository->getBlock().getHash(); }

        Block getBlock() const { return m_repository->getBlock(); }

        void setBlockNumber(uint64_t blockNumber) { m_blockNumber = blockNumber; }

        bool isDone() const { return m_isDone; }

        void setDone() { m_isDone = true; }

        void setParentEmpty() { m_repository->setParentNULL(); }

        void commit() { m_repository->commit(); }

        bool m_isDone = false;
        uint64_t m_blockNumber;
        std::shared_ptr<runtime::storage::Repository> m_repository;
    };

    typedef core::Queue<MemoryItem*> Queue_t;

public:
    BlockChain(crypto::GKey const& key, DatabaseController* dbc, BlockChainMessageFace *messageFace, ChainID const& chainID = DEFAULT_GSE_NETWORK);

    virtual ~BlockChain();

    void init();

    void pushSchedule();

    void initializeRollbackState();

    DatabaseController* getDBC() const { return m_dbc; }

    ChainID const& getChainID() const { return m_chainID; }

    BlockChainStatus getBlockChainStatus() const { return m_blockChainStatus; }

    void setChainID(ChainID const& chainID) { m_chainID = chainID; }

    bool processBlock(std::shared_ptr<Block> block);

    bool processProducerBlock(std::shared_ptr<Block> block);

    bool processTransaction(Block const& block, Transaction const& transaction, MemoryItem* mItem);

    bool processTransaction(Transaction const& transaction, MemoryItem* mItem);

    bool checkBifurcation(std::shared_ptr<Block> block);

    DispatchFace* getDispatcher() const { return m_dispatcher; }

    void processObject(std::unique_ptr<Object> object);

    uint64_t getLastBlockNumber() const;

    uint64_t getLastIrreversibleBlockNumber() const;

    Block getLastBlock() const;

    Block getBlockByNumber(uint64_t number);

    std::shared_ptr<Transaction> getTransactionFromCache();

    std::shared_ptr<Block> getBlockFromCache();

    void onIrreversible(BlockStatePtr bsp);

    Address getExpectedProducer(int64_t timestamp) const;

    Producer getProducer(Address const& address);

    Producers getProducerListFromRepo() const;

    void schedule(int64_t timestamp);

    void updateActiveProducers(std::shared_ptr<Block> block);

public: /// used by rpc
    bool preProcessTx(Transaction& tx);

    bool addRPCTx(Transaction& tx);

    Transaction getTx(TxID const& txID);

    Account getAccount(Address const& address);

    uint64_t getBalance(Address const& address);

    uint64_t getHeight() const;

    uint64_t getSolidifyHeight() const;


public: /// Used by network
    bool preProcessTx(bi::tcp::endpoint const& from, Transaction& tx);

    bool preProcessBlock(bi::tcp::endpoint const& from, Block& block);

    void processTxMessage(bi::tcp::endpoint const& from, Transaction& tx);

    void processBlockMessage(bi::tcp::endpoint const& from, Block& block);

    void processSyncBlockMessage(bi::tcp::endpoint const& from, Block& block);

    void processConfirmationMessage(bi::tcp::endpoint const& from, HeaderConfirmation& confirmation);

    void processStatusMessage(bi::tcp::endpoint const& from, Status& status);

    bool isExist(Transaction& tx);

    bool isExist(Block& block);

private:
    MemoryItem* addMemoryItem(std::shared_ptr<Block> block);
    void cancelMemoryItem();

    void commitBlockState(std::shared_ptr<Block> block);
    void popBlockState();

    void doProcessBlock(std::shared_ptr<Block> block);

public:
    void start();

    void stop();

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

    mutable Mutex x_txCache;
    TxCacheMultiIndexType m_txCache;

    mutable Mutex x_blockCache;
    BlockCacheMultiIndexType m_blockCache;

    ProducerSnapshot m_currentActiveProducers;

    ProducerSnapshot m_prevPS;
    ProducerSnapshot m_currentPS;
};
} // end namespace
