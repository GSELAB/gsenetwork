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

#include <producer/ProducerServer.h>
#include <producer/Schedule.h>
#include <config/Constant.h>
#include <utils/Utils.h>
#include <core/JsonHelper.h>

using namespace utils;
using namespace core;

namespace producer {

ProducerServer::ProducerServer(crypto::GKey const& key, ProcuderEventHandleFace* eventHandle, chain::ChainID chainID):
    Task("GSE-PRODUCER", 0), m_key(key), m_eventHandle(eventHandle), m_state(Ready), m_chainID(chainID)
{
    if (m_chainID == GSE_UNKNOWN_NETWORK) {
        throw ProducerException("Unknown Chain ID:" + toString(GSE_UNKNOWN_NETWORK));
    }
}

ProducerServer::~ProducerServer()
{
    CINFO << "ProducerServer::~ProducerServer";
    if (isWorking())
        stopWorking();
    stop();
    terminate();
}

void ProducerServer::start()
{
    if (m_state != Ready) {
        CWARN << "ProducerServer::start failed due to m_state(" << m_state << ")";
        return;
    }

    startWorking();
    if (isWorking()) return;

    CWARN << "Start producer failed!";
    doneWorking();
}

void ProducerServer::suspend()
{
    if (m_state != Running)
        return;

    m_state = Suspend;
}

void ProducerServer::stop()
{
    m_state = Stop;
}

bool ProducerServer::checkProducer(int64_t timestamp) const
{
    unsigned producerPosition = ((timestamp - GENESIS_TIMESTAMP) %
                (TIME_PER_ROUND)) / (PRODUCER_INTERVAL);
    if (m_key.getAddress() == m_schedule.getAddress(producerPosition)) {
        if (((timestamp / PRODUCER_INTERVAL) * PRODUCER_INTERVAL > m_prevTimestamp) ||
            ((1 + timestamp / PRODUCER_INTERVAL) * PRODUCER_INTERVAL <= m_prevTimestamp)) {
            if (m_eventHandle->getBlockChainStatus() == chain::ProducerStatus) {
                return true;
            }
        }
    }

    return false;
}

void ProducerServer::doWork()
{
    unsigned i;

    int64_t timestamp = currentTimestamp();
    if (checkProducer(timestamp)) {
        CINFO << "ProducerServer::doWork - mytime";
        m_prevTimestamp = timestamp;
    } else {
        CINFO << "ProducerServer::doWork - sleep";
        sleepMilliseconds(PRODUCER_SLEEP_INTERVAL);
        return;
    }

    Block prevBlock = m_eventHandle->getLastBlock();
    BlockHeader blockHeader(prevBlock.getNumber() + 1);
    blockHeader.setProducer(m_key.getAddress());
    blockHeader.setChainID(m_chainID);
    blockHeader.setTimestamp(timestamp);
    //blockHeader.setExtra();
    if (prevBlock != EmptyBlock) {
        blockHeader.setParentHash(prevBlock.getHash());
    } else {
        return;
    }

    std::shared_ptr<Block> block = std::make_shared<Block>(blockHeader);
    for (i = 0; i < MAX_TRANSACTIONS_PER_BLOCK; i++) {
        std::shared_ptr<Transaction> transaction = m_eventHandle->getTransactionFromCache();
        if (!transaction) {
            break;
        }

        if (m_eventHandle->checkTransactionNotExisted(transaction->getHash())) {
            block->addTransaction(*transaction);
        }
    }

    block->setRoots();
    block->sign(m_key.getSecret());
    unsigned producerPosition = ((timestamp - GENESIS_TIMESTAMP) %
                (TIME_PER_ROUND)) / (PRODUCER_INTERVAL);
    CWARN << "Generate block - idx:" << producerPosition  << toJson(*block);
    m_eventHandle->broadcast(block);
}
}
