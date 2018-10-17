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
#include <net/Network.h>
#include <core/Log.h>
#include <chain/Types.h>
#include <config/NetConfig.h>

namespace net {

NetController::NetController(crypto::GKey const& key, DispatchFace* dispatcher): m_key(key), m_inited(false)
{
    m_dispatcher = dispatcher;
}

NetController::NetController(crypto::GKey const& key, DispatchFace* dispatcher, config::NetConfig const& netConfig): m_key(key), m_inited(false)
{
    m_dispatcher = dispatcher;
}

NetController::~NetController()
{
    CINFO << "NetController::~NetController";
    if (!m_host)
        delete m_host;
}

void NetController::init()
{
    if (!m_inited) {
        NetworkConfig conf(DEFAULT_LOCAL_IP, DEFAULT_LISTEN_PORT, false);
        m_host = new Host("GSE V1.0", m_key, conf, chain::DEFAULT_GSE_NETWORK);
        if (m_dispatcher)
            m_host->addDispatcher(m_dispatcher);

        m_host->start();
        CINFO << "NetController::init listen port:" << m_host->listenPort();
        m_inited = true;

        bi::tcp::endpoint ep = Network::resolveHost(DEFAULT_LOCAL_IP_PORT);
        m_nodeIPEndpoint = NodeIPEndpoint(ep.address(), ep.port(), ep.port());
        {
            addNode("127.0.0.1:60606");
            addNode("127.0.0.1:60607");
        }

    }
}

void NetController::broadcast(char *msg)
{

}

void NetController::broadcast(std::shared_ptr<core::Transaction> tMsg)
{
    Peers ps = m_host->getPeers();
    CINFO << "NetController::broadcast tx, peers:" << ps.size();
    for (auto i : ps) {

    }
}

void NetController::broadcast(core::Transaction const& tMsg)
{
    Peers ps = m_host->getPeers();
    CINFO << "NetController::broadcast tx, peers:" << ps.size();
    for (auto i : ps) {

    }
}

void NetController::broadcast(std::shared_ptr<core::Block> bMsg)
{
    CINFO << "Net broadcast block(" << bMsg->getNumber() << ")";


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

void NetController::addNode(std::string const& host)
{

    addNode(NodeID(), Network::resolveHost(host));
}

void NetController::addNode(bi::tcp::endpoint const& ep)
{
    addNode(NodeID(), ep);
}

void NetController::addNode(NodeID const& nodeID, bi::tcp::endpoint const& ep)
{
    NodeIPEndpoint nep(ep.address(), ep.port(), ep.port());
    if (m_nodeIPEndpoint == nep) {
        CINFO << "Add current node(" << nep << ") to NodeTable, reject it!";
        return;
    }

    m_host->addNode(nodeID, nep);
}

} // end of namespace