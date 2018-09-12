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

namespace producer {

ProducerServer::~ProducerServer()
{
    if (isWorking()) {
        CWARN << "~ProducerServer stop working";
        stopWorking();
    }

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
            // m_prevTimestamp = timestamp;
            m_prevTimestamp = m_prevTimestamp + PRODUCER_INTERVAL;
        } else {
            // CINFO << "Try to sleep " << PRODUCER_SLEEP_INTERVAL;
            sleepMilliseconds(PRODUCER_SLEEP_INTERVAL);
            return;
        }
    }

    CINFO << "Try to genereate block... current(" << timestamp << ")" << " time(" << m_prevTimestamp << ")";
    std::shared_ptr<Block> block = std::make_shared<Block>();

    /*
    while (block->getSize() < MAX_BLOCK_SIZE) {
        //std::shared_ptr<bundle::TransactionBundle> transaction = netController->getTransactionFromCache();

    }
    */

}



} // end of namespace