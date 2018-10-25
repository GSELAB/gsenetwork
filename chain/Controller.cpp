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
#include <chain/Common.h>

using namespace database;
using namespace net;
using namespace producer;
using namespace rpc;
using namespace core;

namespace chain {

#define RLP_STREAM_PTR_SEND_EXCEPT(name, type, except) do {    \
    RLPStream rlpStream;    \
    name->streamRLP(rlpStream);     \
    m_net->send(rlpStream.out(), type, except);  \
} while (0)

#define RLP_STREAM_SEND_EXCEPT(name, type, except) do {    \
    RLPStream rlpStream;    \
    name.streamRLP(rlpStream);  \
    m_net->send(rlpStream.out(), type, except);  \
} while (0)

#define RLP_STREAM_PTR_SEND(name, type) do {    \
    RLPStream rlpStream;    \
    name->streamRLP(rlpStream);     \
    m_net->send(rlpStream.out(), type);  \
} while (0)

#define RLP_STREAM_SEND(name, type) do {    \
    RLPStream rlpStream;    \
    name.streamRLP(rlpStream);  \
    m_net->send(rlpStream.out(), type);  \
} while (0)

void Controller::init(crypto::GKey const& key)
{
    m_key = key;

    CINFO << "Start database init...";
    m_dbc = new DatabaseController();
    m_dbc->init();

    CINFO << "Start GSE Chain init...";
    m_chain = new BlockChain(key, m_dbc, this);
    m_chain->init();
    m_chain->start();

    CINFO << "Start network init...";
    m_net = new NetController(m_key, m_chain->getDispatcher());
    m_net->init();

    if (ARGs.m_producerON) {
        CINFO << "Start produce...";
        m_producerServer = new ProducerServer(m_key, this);
        m_producerServer->start();
    }

    if (ARGs.m_rpcON) {
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
    if (ARGs.m_producerON && m_producerServer) delete m_producerServer;
    if (ARGs.m_rpcON && m_rpcServer) delete m_rpcServer;

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

/// Producer interface
void Controller::broadcast(std::shared_ptr<Block> block)
{
    m_chain->processProducerBlock(block);
    m_net->broadcast(block);
}

/// Producer interface
void Controller::processProducerEvent()
{

}

// RPC
void Controller::broadcast(Transaction& tx)
{
    if (!m_chain->addRPCTx(tx)) {
        CINFO << "Pre process failed!";
        return;
    }

    RLP_STREAM_SEND(tx, TransactionPacket);
}

Block Controller::getBlockByNumber(uint64_t number)
{
    return m_chain->getBlockByNumber(number);
}

void Controller::broadcast(bi::tcp::endpoint const& from, Block& block)
{
    RLP_STREAM_SEND_EXCEPT(block, BlockPacket, from);
}

void Controller::broadcast(bi::tcp::endpoint const& from, BlockPtr block)
{
    RLP_STREAM_PTR_SEND_EXCEPT(block, BlockPacket, from);
}

void Controller::broadcast(bi::tcp::endpoint const& from, Transaction& tx)
{
    RLP_STREAM_SEND_EXCEPT(tx, TransactionPacket, from);
}

void Controller::broadcast(bi::tcp::endpoint const& from, TransactionPtr tx)
{
    RLP_STREAM_PTR_SEND_EXCEPT(tx, TransactionPacket, from);
}

void Controller::broadcast(bi::tcp::endpoint const& from, HeaderConfirmation& hc)
{
    RLP_STREAM_SEND_EXCEPT(hc, ConfirmationPacket, from);
}

void Controller::broadcast(bi::tcp::endpoint const& from, HeaderConfirmationPtr hcp)
{
    RLP_STREAM_PTR_SEND_EXCEPT(hcp, ConfirmationPacket, from);
}

void Controller::send(HeaderConfirmation& hc)
{
    RLP_STREAM_SEND(hc, ConfirmationPacket);
}

void Controller::send(HeaderConfirmationPtr hcp)
{
    RLP_STREAM_PTR_SEND(hcp, ConfirmationPacket);
}

Controller controller(DEFAULT_GSE_NETWORK);

} /* end namespace */