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
#include <config/Constant.h>
#include <utils/Utils.h>

using namespace utils;

extern Controller controller;

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
    int64_t timestamp = currentTimestamp();
    if (m_prevTimestamp < 0) {
        m_prevTimestamp = timestamp;
    } else {
        if ((timestamp - m_prevTimestamp) >= (PRODUCER_INTERVAL - PRODUCER_SLEEP_INTERVAL / 5)) {
            m_prevTimestamp = m_prevTimestamp + PRODUCER_INTERVAL;
        } else {
            // CINFO << "Try to sleep " << PRODUCER_SLEEP_INTERVAL;
            sleepMilliseconds(PRODUCER_SLEEP_INTERVAL);
            return;
        }
    }

    BlockHeader blockHeader(m_eventHandle->getLastBlockNumber() + 1);
    blockHeader.setProducer(m_key.getAddress());
    // blockHeader.setParentHash();
    blockHeader.setTimestamp(timestamp);
    //blockHeader.setExtra();

    std::shared_ptr<Block> block = std::make_shared<Block>(blockHeader);
    for (unsigned i = 0; i < 20; i++) {
        std::shared_ptr<Transaction> transaction = m_eventHandle->getTransactionFromCache();
        if (transaction) {
            CINFO << "Package transaction to current block(" << block->getNumber() << ")";
            block->addTransaction(*transaction);
        }
    }

    CINFO << "Try to genereate block(number = " << block->getNumber() << ") success -> time(" << m_prevTimestamp << ")";
    // do broadcast opeartion
    m_eventHandle->broadcast(block);


/*
    BlockHeader blockHeader(m_eventHandle->getLastBlockNumber() + 1);
    blockHeader.setProducer(m_key.getAddress());
    // blockHeader.setParentHash();
    blockHeader.setTimestamp(timestamp);
    //blockHeader.setExtra();

    Block block(blockHeader);
    for (unsigned i = 0; i < 20; i++) {
        std::shared_ptr<Transaction> transaction = m_eventHandle->getTransactionFromCache();
        if (transaction) {
            CINFO << "Package transaction to current block(" << block.getNumber() << ")";
            block.addTransaction(*transaction);
        }
    }

    CINFO << "Try to genereate block(number = " << block.getNumber() << ") success -> time(" << m_prevTimestamp << ")";
    // do broadcast opeartion
    {
        m_eventHandle->broadcast(block);
    }

*/
}



} // end of namespace