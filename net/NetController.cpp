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

#include "net/NetController.h"
#include <net/All.h>
#include <core/Log.h>
#include <chain/Types.h>
#include <config/NetConfig.h>

namespace net {

NetController::~NetController()
{
    CINFO << "NetController::~NetController";
    if (!m_host)
        delete m_host;
}

void NetController::broadcast(char *msg)
{

}

void NetController::broadcast(std::shared_ptr<core::Transaction> tMsg)
{

}

void NetController::init()
{
    if (!m_inited) {
        NetworkConfig conf(DEFAULT_LOCAL_IP, DEFAULT_LISTEN_PORT, false);
        m_host = new Host("GSE V1.0", m_key, conf, chain::DEFAULT_GSE_NETWORK);
        m_host->start();
        CINFO << "NetController::init listen port:" << m_host->listenPort();
        m_inited = true;
    }
}

void NetController::broadcast(std::shared_ptr<core::Block> bMsg)
{

}

std::shared_ptr<core::Transaction> NetController::getTransactionFromCache()
{
    std::shared_ptr<core::Transaction> ret = nullptr;

    if (!this->transactionsQueue.empty()) {
        ret = this->transactionsQueue.front();
        this->transactionsQueue.pop();
    }

    return ret;
}

std::shared_ptr<core::Block> NetController::getBlockFromCache()
{
    std::shared_ptr<core::Block> ret = nullptr;

    if (!transactionsQueue.empty()) {
        ret = blocksQueue.front();
        blocksQueue.pop();
    }

    return ret;
}

} // end of namespace