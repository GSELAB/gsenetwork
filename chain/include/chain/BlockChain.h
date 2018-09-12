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

using namespace net;

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

class BlockChain {
public:
    struct MemoryItem {
        MemoryItem(): m_isDone(false) {}

        MemoryItem(uint64_t number, std::shared_ptr<runtime::storage::Repository> repository): m_isDone(false), m_blockNumber(number), m_repository(repository) {}

        std::shared_ptr<runtime::storage::Repository> getRepository() const { return m_repository; }

        void setRepository(std::shared_ptr<runtime::storage::Repository> repository) { m_repository = repository; }

        uint64_t getBlockNumber() const { return m_blockNumber; }

        void setBlockNumber(uint64_t blockNumber) { m_blockNumber = blockNumber; }

        bool isDone() const { return m_isDone; }

        void setDone() { m_isDone = true; }

        bool m_isDone = false;
        uint64_t m_blockNumber;
        std::shared_ptr<runtime::storage::Repository> m_repository;
    };

public:
    BlockChain(crypto::GKey const& key);

    BlockChain(crypto::GKey const& key, Controller* c, ChainID const& chainID = DEFAULT_GSE_NETWORK);

    virtual ~BlockChain() {}

    void init();

    Controller* getController() const { return m_controller; }

    ChainID const& getChainID() const { return m_chainID; }

    void setChainID(ChainID const& chainID) { m_chainID = chainID; }

    bool processBlock(core::Block const& block);

    bool processTransaction(core::Block const& block, Transaction const& transaction);

    bool processTransaction(core::Transaction const& transaction);

    bool checkBifurcation();

    DispatchFace* getDispatcher() const { return m_dispatcher; }

    void processObject(std::unique_ptr<core::Object> object);

private:

    Controller* m_controller = nullptr;

    GKey m_key;
    ChainID m_chainID = GSE_UNKNOWN_NETWORK;

    DispatchFace* m_dispatcher;

    mutable Mutex x_memoryQueue;
    std::queue<MemoryItem> m_memoryQueue;
    uint64_t m_solidifyIndex;
};
} // end namespace