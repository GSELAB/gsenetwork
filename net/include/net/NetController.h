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
    NetController(crypto::GKey const& key, DispatchFace* dispatcher, chain::ChainID chainID = GSE_UNKNOWN_NETWORK);

    NetController(crypto::GKey const& key, DispatchFace* dispatcher, config::NetConfig const& netConfig, chain::ChainID chainID = GSE_UNKNOWN_NETWORK);

    ~NetController();

    void init();

    void stop();

    void broadcast(char *msg);

    void broadcast(std::shared_ptr<core::Transaction> tMsg);

    void broadcast(core::Transaction const& tMsg);

    void broadcast(std::shared_ptr<core::Block> bMsg);

protected:
    void addNode(std::string const& host);

    void addNode(bi::tcp::endpoint const& ep);

    void addNode(NodeID const& nodeID, bi::tcp::endpoint const& ep);

    core::RLPStream& prepare(core::RLPStream& rlpStream, unsigned id, unsigned args);

public:
    void send(bytes const& data, chain::ProtocolPacketType packetType);

    void send(bytes const& data, chain::ProtocolPacketType packetType, bi::tcp::endpoint const& except);

    void send(bytes const& data, bi::tcp::endpoint const& to, chain::ProtocolPacketType packetType);

private:
    chain::ChainID m_chainID;

    bool m_inited;
    crypto::GKey m_key;

    DispatchFace* m_dispatcher;
    NetworkConfig m_networkConfig;
    NodeIPEndpoint m_nodeIPEndpoint;
    Host* m_host;

    std::shared_ptr<Client> m_client;
};

} // end of namespace