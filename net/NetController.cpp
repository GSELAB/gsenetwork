#include "net/NetController.h"
#include <net/All.h>
#include <core/Log.h>
#include <chain/Types.h>
#include <crypto/Valid.h>
#include <config/Argument.h>

using namespace config;
using namespace chain;

namespace net {

NetController::NetController(crypto::GKey const& key, DispatchFace* dispatcher, chain::ChainID chainID):
    m_key(key), m_inited(false), m_dispatcher(dispatcher),
    m_networkConfig(ARGs.m_local.m_address.to_string(), ARGs.m_local.m_tcpPort, false),
    m_host(new Host("GSE V1.0", m_networkConfig)), m_chainID(chainID)
{
    if (m_chainID == GSE_UNKNOWN_NETWORK) {
        throw NetControllerException("Unknown Chain ID:" + toString(GSE_UNKNOWN_NETWORK));
    }
}

NetController::NetController(crypto::GKey const& key, DispatchFace* dispatcher, config::NetConfig const& netConfig, chain::ChainID chainID):
    NetController(key, dispatcher, chainID)
{
}

NetController::~NetController()
{
    CINFO << "NetController::~NetController";
    delete m_host;
}

void NetController::init()
{
    if (!m_inited) {
        if (m_dispatcher) m_host->setDispatcher(m_dispatcher);
        m_client = std::make_shared<Client>(*m_host, m_dispatcher);
        m_host->registerCapability(m_client);
        m_host->start();
        m_client->start();
        m_inited = true;
        m_nodeIPEndpoint = NodeIPEndpoint(ARGs.m_local.m_address, ARGs.m_local.m_tcpPort, ARGs.m_local.m_udpPort);
        if (ARGs.m_neighbors.size() > 0)
            for (auto i : ARGs.m_neighbors)
                addNode(NodeIPEndpoint(i.m_address, i.m_tcpPort, i.m_udpPort));

        if (ARGs.m_trustNeighbors.size() > 0)
            for (auto i : ARGs.m_trustNeighbors)
                addNode(NodeIPEndpoint(i.m_address, i.m_tcpPort, i.m_udpPort));
    }
}

void NetController::stop()
{
    m_client->stop();
    m_host->stop();
    m_client.reset();
}

void NetController::broadcast(char *msg)
{

}

void NetController::broadcast(std::shared_ptr<core::Transaction> tMsg)
{
    broadcast(*tMsg);
}

void NetController::broadcast(core::Transaction const& tMsg)
{
    core::RLPStream rlpStream;
    tMsg.streamRLP(rlpStream);
    bytes data = rlpStream.out();
    send(data, TransactionPacket);
}

void NetController::broadcast(std::shared_ptr<core::Block> bMsg)
{
    core::RLPStream rlpStream;
    bMsg->streamRLP(rlpStream);
    bytes data = rlpStream.out();
    send(data, BlockPacket);
}

void NetController::addNode(std::string const& host)
{

    addNode(NodeID(), Network::resolveHost(host));
}

void NetController::addNode(bi::tcp::endpoint const& ep)
{
    addNode(NodeID(), ep);
}

void NetController::addNode(NodeID const& nodeID, bi::tcp::endpoint const& ep)
{
    NodeIPEndpoint nep(ep.address(), ep.port(), ep.port());
    if (m_nodeIPEndpoint == nep) {
        CINFO << "Add current node(" << nep << ") to NodeTable, reject it!";
        return;
    }

    m_host->addNode(nodeID, nep);
}

core::RLPStream& NetController::prepare(core::RLPStream& rlpStream, unsigned id, unsigned args)
{
    return rlpStream.appendRaw(bytes(1, id)).appendList(args);
}

void NetController::send(bytes const& data, ProtocolPacketType packetType)
{
    Peers ps = m_host->getPeers();
    for (auto i : ps) {
        NodeID id = i.address();
        std::shared_ptr<SessionFace> session = m_host->peerSession(id);
        if (session) {
            core::RLPStream rlpStream;
            prepare(rlpStream, packetType, 1).appendRaw(data);
            session->sealAndSend(rlpStream);
        }
    }
}

void NetController::send(bytes const& data, ProtocolPacketType packetType, bi::tcp::endpoint const& except)
{
    Peers ps = m_host->getPeers();
    for (auto i : ps) {
        NodeID id = i.address();
        if (except.address() == i.endpoint.address() && except.port() == i.endpoint.tcpPort())
            continue;
        std::shared_ptr<SessionFace> session = m_host->peerSession(id);
        if (session) {
            core::RLPStream rlpStream;
            prepare(rlpStream, packetType, 1).appendRaw(data);
            session->sealAndSend(rlpStream);
        }
    }
}

void NetController::send(bytes const& data, bi::tcp::endpoint const& to, chain::ProtocolPacketType packetType)
{
    Peers ps = m_host->getPeers();
    for (auto i : ps) {
        NodeID id = i.address();
        if (to.address() == i.endpoint.address() && to.port() == i.endpoint.tcpPort()) {
            std::shared_ptr<SessionFace> session = m_host->peerSession(id);
            if (session) {
                core::RLPStream rlpStream;
                prepare(rlpStream, packetType, 1).appendRaw(data);
                session->sealAndSend(rlpStream);
            }

            break;
        }
    }
}
}