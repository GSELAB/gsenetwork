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

#include <chain/Types.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace net {

class BytesSocket : public std::enable_shared_from_this<BytesSocket> {
public:
    BytesSocket(boost::asio::io_service& ioService, chain::ChainId id);
    ~BytesSocket();

    bool isConnected() const;
    void close();
    boost::asio::ip::tcp::endpoint remoteEndpoint();
    boost::asio::ip::tcp::socket& ref();

    void setChainId(chain::ChainId id);
    chain::ChainId getChainId() const;

protected:
    chain::ChainId m_chainId;
    boost::asio::ip::tcp::socket m_socket;
};

} // end namespace