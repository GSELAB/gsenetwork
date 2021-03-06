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
#include <utils/Utils.h>

using namespace database;
using namespace net;
using namespace producer;
using namespace rpc;
using namespace core;
using namespace utils;

namespace chain {

#define RLP_STREAM_PTR_SEND_EXCEPT(name, type, except) \
do {    \
    RLPStream rlpStream;    \
    name->streamRLP(rlpStream);     \
    if (m_net != nullptr)   \
        m_net->send(rlpStream.out(), type, except);  \
} while (0)

#define RLP_STREAM_SEND_EXCEPT(name, type, except) \
do {    \
    RLPStream rlpStream;    \
    name.streamRLP(rlpStream);  \
    if (m_net != nullptr)   \
        m_net->send(rlpStream.out(), type, except);  \
} while (0)

#define RLP_STREAM_PTR_SEND_TO(name, type, to) \
do {    \
    RLPStream rlpStream;    \
    name->streamRLP(rlpStream); \
    if (m_net != nullptr)   \
        m_net->send(rlpStream.out(), to, type); \
} while (0)

#define RLP_STREAM_SEND_TO(name, type, to) \
do {    \
    RLPStream rlpStream;    \
    name.streamRLP(rlpStream); \
    if (m_net != nullptr)   \
        m_net->send(rlpStream.out(), to, type); \
} while (0)

#define RLP_STREAM_PTR_SEND(name, type) \
do {    \
    RLPStream rlpStream;    \
    name->streamRLP(rlpStream);     \
    if (m_net != nullptr)   \
        m_net->send(rlpStream.out(), type);  \
} while (0)

#define RLP_STREAM_SEND(name, type) \
do {    \
    RLPStream rlpStream;    \
    name.streamRLP(rlpStream);  \
    if (m_net != nullptr)   \
        m_net->send(rlpStream.out(), type);  \
} while (0)

Controller controller;

void Controller::init(crypto::GKey const& key, ChainID chainID)
{
    m_chainID = chainID;
    m_key = key;
    m_dbc = new DatabaseController();
    m_dbc->init();

    m_chain = new BlockChain(key, m_dbc, this, m_chainID);
    m_chain->init();
    m_chain->start();

    m_net = new NetController(m_key, m_chain->getDispatcher(), m_chainID);
    m_net->init();

    m_producerServer = new ProducerServer(m_key, this, m_chainID);
    m_chain->pushSchedule();
    CINFO << "Producer start - " << ARGs.m_producerON;
    if (ARGs.m_producerON) {
        m_producerServer->start();
    }

    if (ARGs.m_rpcON) {
        m_rpcServer = new RpcService(this, m_chainID);
        m_rpcServer->start();
    }
}

void Controller::exit()
{
    CINFO << "Controller release the resource...";
    m_chain->setBlockChainStatus(BlockChainStatus::Killed);
    m_chain->stop();
    if (ARGs.m_rpcON && m_rpcServer != nullptr) {
        delete m_rpcServer;
    }

    if (ARGs.m_producerON && m_producerServer != nullptr) {
        m_producerServer->stop();
        sleepMilliseconds(500);
        delete m_producerServer;
        m_producerServer = nullptr;
    }

    m_net->stop();
    sleepMilliseconds(500);
    m_net = nullptr;
    m_chain = nullptr;
    m_dbc = nullptr;
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
    if (m_chain == nullptr) {
        return;
    }

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
    if (m_chain == nullptr) {
        return;
    }

    if (!m_chain->addRPCTx(tx)) {
        // CINFO << "Pre process failed!";
        return;
    }

    RLP_STREAM_SEND(tx, TransactionPacket);
}

Block Controller::getBlockByNumber(uint64_t number)
{
    if (m_chain == nullptr) {
        return EmptyBlock;
    }

    return m_chain->getBlockByNumber(number);
}

void Controller::broadcast(boost::asio::ip::tcp::endpoint const& from, Block& block)
{
    RLP_STREAM_SEND_EXCEPT(block, BlockPacket, from);
}

void Controller::broadcast(boost::asio::ip::tcp::endpoint const& from, BlockPtr block)
{
    RLP_STREAM_PTR_SEND_EXCEPT(block, BlockPacket, from);
}

void Controller::broadcast(boost::asio::ip::tcp::endpoint const& from, Transaction& tx)
{
    RLP_STREAM_SEND_EXCEPT(tx, TransactionPacket, from);
}

void Controller::broadcast(boost::asio::ip::tcp::endpoint const& from, TransactionPtr tx)
{
    RLP_STREAM_PTR_SEND_EXCEPT(tx, TransactionPacket, from);
}

void Controller::broadcast(boost::asio::ip::tcp::endpoint const& from, HeaderConfirmation& hc)
{
    RLP_STREAM_SEND_EXCEPT(hc, ConfirmationPacket, from);
}

void Controller::broadcast(boost::asio::ip::tcp::endpoint const& from, HeaderConfirmationPtr hcp)
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

void Controller::send(boost::asio::ip::tcp::endpoint const& to, Status& status)
{
    RLP_STREAM_SEND_TO(status, StatusPacket, to);
}

void Controller::send(boost::asio::ip::tcp::endpoint const& to, StatusPtr status)
{
    RLP_STREAM_PTR_SEND_TO(status, StatusPacket, to);
}

void Controller::send(boost::asio::ip::tcp::endpoint const& to, BlockState& bs)
{
    RLP_STREAM_SEND_TO(bs, BlockStatePacket, to);
}

void Controller::send(boost::asio::ip::tcp::endpoint const& to, BlockStatePtr bsp)
{
    RLP_STREAM_PTR_SEND_TO(bsp, BlockStatePacket, to);
}
} /* end namespace */
