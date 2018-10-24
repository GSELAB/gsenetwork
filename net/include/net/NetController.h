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

#pragma once

#include <queue>

#include <core/Block.h>
#include <core/Transaction.h>
#include <config/NetConfig.h>
#include <crypto/GKey.h>
#include <net/BytesPacket.h>
#include <net/Common.h>
#include <net/Network.h>
#include <net/Client.h>
#include <chain/Common.h>
#include <chain/BlockState.h>

using namespace core;
using namespace chain;

namespace net {

class NetController {
public:
    NetController(crypto::GKey const& key, DispatchFace* dispatcher);

    NetController(crypto::GKey const& key, DispatchFace* dispatcher, config::NetConfig const& netConfig);

    ~NetController();

    void init();

    void broadcast(char *msg);

    void broadcast(std::shared_ptr<core::Transaction> tMsg);

    void broadcast(core::Transaction const& tMsg);

    void broadcast(std::shared_ptr<core::Block> bMsg);

public: // used by block chain
    virtual void broadcast(bi::tcp::endpoint const& from, Block& block);

    virtual void broadcast(bi::tcp::endpoint const& from, BlockPtr block);

    virtual void broadcast(bi::tcp::endpoint const& from, Transaction& tx);

    virtual void broadcast(bi::tcp::endpoint const& from, TransactionPtr tx);

    virtual void broadcast(bi::tcp::endpoint const& from, BlockState& bs);

    virtual void broadcast(bi::tcp::endpoint const& from, BlockStatePtr bsp);

    virtual void send(BlockState& bs);

    virtual void send(BlockStatePtr bsp);

protected:
    void addNode(std::string const& host);

    void addNode(bi::tcp::endpoint const& ep);

    void addNode(NodeID const& nodeID, bi::tcp::endpoint const& ep);

    void send(bytes const& data, chain::ProtocolPacketType packetType);

    core::RLPStream& prepare(core::RLPStream& rlpStream, unsigned id, unsigned args);

private:
    bool m_inited;
    crypto::GKey m_key;

    DispatchFace* m_dispatcher;
    NetworkConfig m_networkConfig;
    NodeIPEndpoint m_nodeIPEndpoint;
    Host* m_host;
};

} // end of namespace