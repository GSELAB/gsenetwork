#pragma once

#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <crypto/Common.h>
#include <chain/Genesis.h>

namespace ba = boost::asio;
namespace bi = ba::ip;

namespace config {

struct Host {
    Host() {}
    Host(bi::address const& address, uint16_t tcpP, uint16_t udpP): m_address(address), m_tcpPort(tcpP), m_udpPort(udpP) {}
    ~Host() {}

    Host& operator=(Host const& h) { if (this == &h) return *this; m_address = h.m_address; m_tcpPort = h.m_tcpPort; m_udpPort = h.m_udpPort; return *this; }

    bool operator==(Host const& h) const { return m_address == h.m_address && m_tcpPort == h.m_tcpPort && m_udpPort == h.m_udpPort; }

    bool operator!=(Host const& h) const { return !operator==(h); }

    bi::address m_address;
    uint16_t m_tcpPort;
    uint16_t m_udpPort;
};

class Argument {
public:
    bool m_producerON = false;
    bool m_rpcON = true;

    Host m_local;
    Host m_rpc;
    std::vector<Host> m_neighbors;
    std::vector<Host> m_trustNeighbors;

    crypto::Secret m_secret;

    bool m_syncFlag = true;

    chain::Genesis m_genesis;
};

extern Argument ARGs;

void initArgument();
}