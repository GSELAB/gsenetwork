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

    {
        // Just for testing
        Producer self(m_key.getAddress(), currentTimestamp());
        self.setVotes(1000);
        m_schedule.addProducer(self);
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

bool ProducerServer::checkProducer(int64_t timestamp) const
{
    unsigned producerPosition = ((timestamp - GENESIS_TIMESTAMP) %
                (PRODUCER_INTERVAL * NUM_DELEGATED_BLOCKS)) / (PRODUCER_INTERVAL);
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
        m_prevTimestamp = timestamp;
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
            CINFO << "Package transaction to current block(" << block->getNumber() << ")";
            block->addTransaction(*transaction);
        }
    }

    {
        // Just test
        Transaction tx(DEFAULT_GSE_NETWORK, Transaction::TransferType, m_key.getAddress(), Address("969956599be0D545812813F76C61B2D9E283cc7c"),
            currentTimestamp(), bytes(), 10);
        tx.sign(m_key.getSecret());
        block->addTransaction(tx);
    }
    CINFO << "Add to block";

    // set receipts
    {
        //size_t transactionCount = block.getTransactionsSize();
        //for (i = 0; i < transactionCount; i++) {
        //
        //}
    }

    block->setRoots();
    block->sign(m_key.getSecret());
    CINFO << "Generate block:" << toJson(*block);
    m_eventHandle->broadcast(block);
}
} // end of namespace
