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

#include <net/BytesHandshake.h>
#include <net/Host.h>
#include <net/Session.h>
#include <net/Peer.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

using namespace std;
using namespace core;
using namespace net;
using namespace crypto;

void BytesHandshake::writeAuth()
{
    // send auth to the remote node
}

void BytesHandshake::writeAck()
{
    // send ack to the remote node
}

void BytesHandshake::writeAckEIP8()
{
    // send EIP-8 format ack to remote node
}

//Signature const& sig, Public const& remotePubk, h256 const& remoteNonce, uint64_t remoteVersion);
void BytesHandshake::setAuthValues(Signature const& sig, Public const& remotePubk,
    h256 const& remoteNonce, uint64_t remoteVrsion)
{

}

void BytesHandshake::readAuth()
{
    // receive auth from remote node
}

void BytesHandshake::readAuthEIP8()
{
    // receive auth in EIP-8 format from remote node
}

void BytesHandshake::readAck()
{
    // receive ack from remote node
}

void BytesHandshake::readAckEIP8()
{
    // receive EIP-8 format ack from remote node
}

void BytesHandshake::start()
{
    transition();
}

void BytesHandshake::cancel()
{
    // cancel the handshake
}

void BytesHandshake::transition(boost::system::error_code ec)
{
    // reset timeout
    m_idleTimer.cancel();

    if (ec || m_nextState == Error || m_cancel) {
        // Error occur
        return;
    }

    auto self(shared_from_this());
    assert(m_nextState != StartSession);
    m_idleTimer.expires_from_now(c_timeout);
    m_idleTimer.async_wait([this, self](boost::system::error_code const& _ec) {
        if (!_ec)
        {
            if (!m_socket->remoteEndpoint().address().is_unspecified())
                LOG(m_logger) << "Disconnecting " << m_socket->remoteEndpoint()
                              << " (Handshake Timeout)";
            cancel();
        }
    });

    // process state
    if (m_nextState == New) {
        m_nextState = AckAuth;


    } else if (m_nextState == AckAuth) {
        m_nextState = WriteHello;


    } else if (m_nextState == AckAuthEIP8) {
        m_nextState = WriteHello;


    } else if (m_nextState == WriteHello) {
        m_nextState = ReadHello;


    } else if (m_nextState == ReadHello) {
        m_nextState = StartSession;

    }

}

