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

#include <boost/thread.hpp>

#include <chain/Controller.h>
#include <core/Log.h>
#include <config/Argument.h>

using namespace database;
using namespace net;
using namespace producer;
using namespace rpc;
using namespace core;

namespace chain {

void Controller::init(crypto::GKey const& key)
{
    m_key = key;

    CINFO << "Start database init...";
    m_dbc = new DatabaseController();
    m_dbc->init();

    CINFO << "Start GSE Chain init...";
    m_chain = new BlockChain(key, this);
    m_chain->init();

    CINFO << "Start network init...";
    m_net = new NetController(m_key, m_chain->getDispatcher());
    m_net->init();

    if (argInstance.m_producerON) {
        CINFO << "Start produce...";
        m_producerServer = new ProducerServer(m_key, this);
        m_producerServer->start();
    }

    if (argInstance.m_rpcON) {
        CINFO << "Start rpc service ...";
        m_rpcServer = new RpcService(this);
        m_rpcServer->start();
    }

}

void Controller::exit()
{
    CINFO << "Controller release the resource...";
    if (m_net) delete m_net;
    if (m_chain) delete m_chain;
    if (m_dbc) delete m_dbc;
    if (argInstance.m_producerON && m_producerServer) delete m_producerServer;
    if (argInstance.m_rpcON && m_rpcServer) delete m_rpcServer;

}

std::shared_ptr<TransactionReceipt> Controller::processTransaction(Transaction const& transaction, int64_t max_timestamp)
{
    return nullptr;
}

void Controller::processBlock(Block const& block)
{

}

void Controller::processTransaction(Transaction const& transaction)
{

}

chain::ChainID Controller::getChainID() const
{
    return m_chainID;
}

void Controller::setChainID(chain::ChainID chainID)
{
    m_chainID = chainID;
}

// @only used by rpc module
bool Controller::generateTransaction()
{
    return true;
}

// @only used by rpc module
bool Controller::addTransaction(Transaction const& transaction)
{
    return true;
}

// @used by producer
void Controller::broadcast(std::shared_ptr<Block> block)
{
    // send to current block chain
    m_chain->processProducerBlock(block);

    // send it to p2p net work
    m_net->broadcast(block);
}

// @used by producer
void Controller::processProducerEvent()
{

}

// used by rpc
void Controller::broadcast(Transaction const& transaction)
{
    // send to current transactions cache

    // send to p2p network
}

Block Controller::getBlockByNumber(uint64_t number)
{
    return m_chain->getBlockByNumber(number);
}


Controller controller(DEFAULT_GSE_NETWORK);

} /* end namespace */