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

namespace net {

NetController::NetController(config::NetConfig const& netConfig): m_inited(false)
{

}

NetController::~NetController()
{
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
    CINFO << "NetController::init : m_inited:" << m_inited;
    if (!m_inited) {

        const char *const localhost = "127.0.0.1";
        net::NetworkConfig conf(localhost, 0, false);

        CINFO << "NetController::init new Host";
        m_host = new Host("GSE V1.0", conf);
        CINFO << "NetController::init host:" << m_host;
        //net::Host *host = new net::Host("GSE V1.0", conf);
        //host->start();
        //host->listenPort();

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