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

#include <vector>
#include <string>

#include <core/Producer.h>
#include <core/Account.h>
#include <core/Task.h>
#include <crypto/GKey.h>
#include <config/Constant.h>
#include <producer/Schedule.h>
#include <chain/BlockChain.h>

using namespace core;

namespace producer {

class ProcuderEventHandleFace {
public:
    virtual ~ProcuderEventHandleFace() {}

    virtual void broadcast(std::shared_ptr<Block> block) = 0;

    virtual void processProducerEvent() = 0;

    virtual uint64_t getLastBlockNumber() const = 0;

    virtual Block getLastBlock() const = 0;

    virtual std::shared_ptr<Transaction> getTransactionFromCache() = 0;

    virtual std::shared_ptr<Block> getBlockFromCache() = 0;

    virtual chain::BlockChainStatus getBlockChainStatus() const = 0;
};

/* The producer's state. */
enum ProducerState {
    Ready,
    Running,
    Suspend,
    Stop,
    Undefined
};

enum ProducerRet {
    PR_SUCCESS,
    PR_FAILED,
    PR_UNDEFINED,
};

class ProducerServer: public Task {
public:
    ProducerServer(crypto::GKey const& key, ProcuderEventHandleFace* eventHandle):
        Task("GSE-PRODUCER", 0), m_key(key), m_eventHandle(eventHandle), m_state(Ready) {}

    virtual ~ProducerServer();

    void start();

    void suspend();

    void stop();

    void doWork();

    bool checkProducer(int64_t timestamp) const;

public:
    void schedule(ProducersConstRef producerList) { m_schedule.schedule(producerList); }

    ProducersConstRef getSchedule() { return m_schedule.getProducerList(); }

    Address getProducerAddress(unsigned idx) const { return m_schedule.getAddress(idx); }

private:
    chain::ChainID m_chainID;
    crypto::GKey m_key;

    //std::vector<std::shared_ptr<Producer>> m_producers;
    ProducerState m_state;
    std::shared_ptr<Account> m_currentAccout;

    ProcuderEventHandleFace* m_eventHandle;

    Schedule m_schedule;

    int64_t m_prevTimestamp = -1;
};

} // end of namespace
