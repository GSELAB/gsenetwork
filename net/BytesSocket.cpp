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

#include <net/BytesSocket.h>

using namespace net;

BytesSocket::BytesSocket(boost::asio::io_service& ioService, chain::ChainID id) : m_socket(ioService)
{
    m_chainId = id;
}

BytesSocket::~BytesSocket()
{

}

bool BytesSocket::isConnected() const
{
    return m_socket.is_open();
}

void BytesSocket::close()
{
    try {
        boost::system::error_code errc;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, errc);
        if (m_socket.is_open()) {
            m_socket.close();
        }
    } catch (...) {

    }
}

boost::asio::ip::tcp::endpoint BytesSocket::remoteEndpoint()
{
    boost::system::error_code errc;
    return m_socket.remote_endpoint(errc);
}

boost::asio::ip::tcp::socket& BytesSocket::ref()
{
    return m_socket;
}

void BytesSocket::setChainId(chain::ChainID id)
{
    m_chainId = id;
}

chain::ChainID BytesSocket::getChainId() const
{
    return m_chainId;
}