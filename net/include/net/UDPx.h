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

#include <memory>

#include <net/Common.h>
#include <core/RLP.h>
#include <core/Log.h>

namespace net {

class UDPDatagram {
public:
    UDPDatagram(boost::asio::ip::udp::endpoint const& ep) :
        locus(ep) {}

    UDPDatagram(boost::asio::ip::udp::endpoint const& ep, bytes in) :
        locus(ep), data(in) {}

    boost::asio::ip::udp::endpoint const& endpoint() const {
        return locus;
    }

    bytes data;
protected:
    boost::asio::ip::udp::endpoint locus;
};

// BytesDatagramFace could be signed
struct BytesDatagramFace : public UDPDatagram {
    static uint32_t futureFromEpoch(std::chrono::seconds _sec) {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>((std::chrono::system_clock::now() + _sec).time_since_epoch()).count());
    }

    static uint32_t secondsSinceEpoch() {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>((std::chrono::system_clock::now()).time_since_epoch()).count());
    }

    static Public authenticate(bytesConstRef sig, bytesConstRef rlp);

    // none static function
    BytesDatagramFace(boost::asio::ip::udp::endpoint const& ep) :
        UDPDatagram(ep) {}
    virtual ~BytesDatagramFace() = default;

    virtual h256 sign(Secret const& sec);
    virtual uint8_t packetType() const = 0;

    virtual void streamRLP(core::RLPStream &) const = 0;
    virtual void interpretRLP(bytesConstRef bs) = 0;
};

// interfce for udp socket
struct UDPSocketFace {
    virtual bool send(UDPDatagram const& msg) = 0;
    virtual void disconnect() = 0;
};

// interface for udp socket's owner
struct UDPSocketEvents {
    virtual ~UDPSocketEvents() = default;
    virtual void onDisconnected(UDPSocketFace *) {}
    virtual void onReceived(UDPSocketFace *, boost::asio::ip::udp::endpoint const& from, bytesConstRef pakcetData) = 0;
};

/**
 * Handler must implement UDPSocketEvents
 *
 */
template<typename Handler, unsigned MaxDatagramSize>
class UDPSocket : UDPSocketFace, public std::enable_shared_from_this<UDPSocket<Handler, MaxDatagramSize>> {
public:
    enum {
        maxDatagramSize = MaxDatagramSize,
    };

    static_assert((unsigned)maxDatagramSize < 65535u, "UDP datagrams cannot be larger than 65535 bytes");

    // specific endpoint
    UDPSocket(boost::asio::io_service& io, UDPSocketEvents& host, boost::asio::ip::udp::endpoint ep) :
        m_host(host), m_endpoint(ep), m_socket(io)
    {
        m_started.store(false);
        m_closed.store(true);
    }

    // listen to all ports
    UDPSocket(boost::asio::io_service& io, UDPSocketEvents& host, unsigned port) :
        m_host(host), m_endpoint(boost::asio::ip::udp::v4, port), m_socket(io)
    {
        m_started.store(false);
        m_closed.store(true);
    }

    virtual ~UDPSocket() {
        disconnect();
    }

    void connect();
    bool send(UDPDatagram const& data);
    bool isOpen() {
        !m_closed;
    }

    void disconnect() {
        disconnectWithError(boost::asio::error::connection_reset);
    }

protected:
    void doRead();
    void doWrite();
    void disconnectWithError(boost::system::error_code ec);

private:
    std::atomic<bool> m_started;
    std::atomic<bool> m_closed;

    UDPSocketEvents& m_host; // interface which owns this socket
    boost::asio::ip::udp::endpoint m_endpoint;

    Mutex x_sendQ;
    std::deque<UDPDatagram> m_sendQ; // queue for egress data
    std::array<byte, maxDatagramSize> m_recvData; // buffer for ingress data
    boost::asio::ip::udp::endpoint m_recvEndpoint; // data from received from
    boost::asio::ip::udp::endpoint m_socket; // boost asio udp socket

    Mutex x_socketError;
    boost::system::error_code m_socketError; // could be set from host or IO thread

    Logger m_logger{createLogger(VerbosityInfo, "udp")};
};

} // end namespace