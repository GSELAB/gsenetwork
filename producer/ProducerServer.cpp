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

    /*
    m_state = Running;
    while (m_state == Running) {
        generateBlock();
    }
    */
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

void ProducerServer::doWork()
{
    unsigned i;

    int64_t timestamp = currentTimestamp();
    unsigned producerPosition = ((timestamp - GENESIS_TIMESTAMP) %
                (PRODUCER_INTERVAL * NUM_DELEGATED_BLOCKS)) / (PRODUCER_INTERVAL);

    const std::vector<Producer> activeProducers = m_schedule.getActiveProducers();
    if (!activeProducers.empty()) {
        if (m_key.getAddress() == activeProducers[producerPosition].getAddress()) {
            if (((timestamp / PRODUCER_INTERVAL) * PRODUCER_INTERVAL > m_prevTimestamp) ||
                ((1 + timestamp / PRODUCER_INTERVAL) * PRODUCER_INTERVAL <= m_prevTimestamp)) {
                if (m_eventHandle->getBlockChainStatus() == chain::ProducerStatus) {
                    m_prevTimestamp = timestamp;
                }
            }
        }
    } else {
        sleepMilliseconds(PRODUCER_SLEEP_INTERVAL);
        return;
    }

    Block prevBlock = m_eventHandle->getLastBlock();
    BlockHeader blockHeader(m_eventHandle->getLastBlockNumber() + 1);
    blockHeader.setProducer(m_key.getAddress());
    if (prevBlock != EmptyBlock) {
        CINFO << "prev:" << toJson(prevBlock).toStyledString();
        blockHeader.setParentHash(prevBlock.getHash());
    }

    blockHeader.setTimestamp(timestamp);
    //blockHeader.setExtra();

    std::shared_ptr<Block> block = std::make_shared<Block>(blockHeader);
    for (i = 0; i < MAX_TRANSACTIONS_PER_BLOCK; i++) {
        std::shared_ptr<Transaction> transaction = m_eventHandle->getTransactionFromCache();
        if (transaction) {
            if (sizeof(*transaction) > MAX_TRANSACTION_SIZE)
                continue;
            CINFO << "Package transaction to current block(" << block->getNumber() << ")";
            block->addTransaction(*transaction);
        }
    }

    // set receipts
    {
        //size_t transactionCount = block.getTransactionsSize();
        //for (i = 0; i < transactionCount; i++) {
        //
        //}
    }

    block->setRoots();
    block->sign(m_key.getSecret());

    // do broadcast opeartion
    m_eventHandle->broadcast(block);
}



} // end of namespace
