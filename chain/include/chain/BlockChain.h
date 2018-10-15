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
#include <core/Transaction.h>
#include <storage/Repository.h>
#include <net/NetController.h>
#include <crypto/GKey.h>
#include <core/Queue.h>
#include <chain/RollbackState.h>

using namespace net;
using namespace core;

namespace chain {

class Controller;
extern Controller controller;

class BlockChain;

class Dispatch: public DispatchFace {
public:
    Dispatch(BlockChain* chain): m_chain(chain) {}

    virtual ~Dispatch() {}

    void processMsg(bi::tcp::endpoint const& from, BytesPacket const& msg);

    static std::unique_ptr<core::Object> interpretObject(bi::tcp::endpoint const& from, BytesPacket const& msg);

private:
    BlockChain *m_chain;
};

enum BlockChainStatus {
    NormalStatus = 0x01,
    SyncStatus,
    ProducerStatus,
};

class BlockChain {
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
    BlockChain(crypto::GKey const& key);

    BlockChain(crypto::GKey const& key, Controller* c, ChainID const& chainID = DEFAULT_GSE_NETWORK);

    virtual ~BlockChain();

    void init();

    void initializeRollbackState();

    Controller* getController() const { return m_controller; }

    ChainID const& getChainID() const { return m_chainID; }

    void setChainID(ChainID const& chainID) { m_chainID = chainID; }

    bool processBlock(std::shared_ptr<Block> block);

    bool processProducerBlock(std::shared_ptr<Block> block);

    bool processTransaction(Block const& block, Transaction const& transaction, MemoryItem* mItem);

    bool processTransaction(Transaction const& transaction, MemoryItem* mItem);

    bool checkBifurcation();

    DispatchFace* getDispatcher() const { return m_dispatcher; }

    void processObject(std::unique_ptr<Object> object);

    uint64_t getLastBlockNumber() const;

    uint64_t getLastIrreversibleBlockNumber() const;

    Block getLastBlock() const;

    Block getBlockByNumber(uint64_t number);

    std::shared_ptr<Transaction> getTransactionFromCache();

    std::shared_ptr<Block> getBlockFromCache();

    void onIrreversible(BlockStatePtr bsp);

private:
    MemoryItem* addMemoryItem(std::shared_ptr<Block> block);
    void cancelMemoryItem();

    void commitBlockState(std::shared_ptr<Block> block);
    void popBlockState();

    void doProcessBlock(std::shared_ptr<Block> block);

private:

    Controller* m_controller = nullptr;

    GKey m_key;
    ChainID m_chainID = GSE_UNKNOWN_NETWORK;

    DispatchFace* m_dispatcher;

    mutable Mutex x_memoryQueue;
    Queue_t m_memoryQueue;

    RollbackState m_rollbackState;
    BlockStatePtr m_head;

    uint64_t m_solidifyIndex;
    BlockChainStatus m_blockChainStatus = NormalStatus;

    // ? Change Mutex to spinlock ??? ?
    mutable Mutex x_transactionsQueue;
    std::queue<std::shared_ptr<core::Transaction>> transactionsQueue;

    mutable Mutex x_blocksQueue;
    std::queue<std::shared_ptr<core::Block>> blocksQueue;
};
} // end namespace