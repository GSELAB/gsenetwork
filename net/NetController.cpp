#include "net/NetController.h"
#include <net/All.h>
#include <net/Network.h>
#include <core/Log.h>
#include <chain/Types.h>
#include <crypto/Valid.h>
#include <config/Argument.h>

using namespace config;

namespace net {

NetController::NetController(crypto::GKey const& key, DispatchFace* dispatcher): m_key(key), m_inited(false)
{
    m_dispatcher = dispatcher;
}

NetController::NetController(crypto::GKey const& key, DispatchFace* dispatcher, config::NetConfig const& netConfig): m_key(key), m_inited(false)
{
    m_dispatcher = dispatcher;
}

NetController::~NetController()
{
    CINFO << "NetController::~NetController";
    if (!m_host)
        delete m_host;
}

void NetController::init()
{
    if (!m_inited) {

        CINFO << "ARGS:" << ARGs.m_local.m_address.to_string() << " port:" <<  ARGs.m_local.m_tcpPort;
        NetworkConfig conf(ARGs.m_local.m_address.to_string(), ARGs.m_local.m_tcpPort, false);
        m_host = new Host("GSE V1.0", m_key, conf); /// , chain::DEFAULT_GSE_NETWORK
        if (m_dispatcher)
            m_host->addDispatcher(m_dispatcher);

        m_host->start();
        CINFO << "NetController listen port:" << m_host->listenPort();
        m_inited = true;
        m_nodeIPEndpoint = NodeIPEndpoint(ARGs.m_local.m_address, ARGs.m_local.m_tcpPort, ARGs.m_local.m_udpPort);

        if (ARGs.m_neighbors.size() > 0) {
            for (auto i : ARGs.m_neighbors) {
                NodeIPEndpoint ep(i.m_address, i.m_tcpPort, i.m_udpPort);
                addNode(ep);
            }
        }

        if (ARGs.m_trustNeighbors.size() > 0) {
            for (auto i : ARGs.m_trustNeighbors) {
                NodeIPEndpoint ep(i.m_address, i.m_tcpPort, i.m_udpPort);
                addNode(ep);
            }
        }
    }
}

void NetController::broadcast(char *msg)
{

}

void NetController::broadcast(std::shared_ptr<core::Transaction> tMsg)
{
    {

    }
    Peers ps = m_host->getPeers();
    for (auto i : ps) {
        NodeID id = i.address();
        std::shared_ptr<SessionFace> session = m_host->peerSession(id);
        if (session) {
            CINFO << "NetController::broadcast tx, find session";
            core::RLPStream rlpStream;
            tMsg->streamRLP(rlpStream);
            session->sealAndSend(rlpStream);
        } else {

        }
    }
}

void NetController::broadcast(core::Transaction const& tMsg)
{
    Peers ps = m_host->getPeers();
    for (auto i : ps) {
        NodeID id = i.address();
        std::shared_ptr<SessionFace> session = m_host->peerSession(id);
        if (session) {
            CINFO << "NetController::broadcast tx, find session";
            core::RLPStream rlpStream;
            tMsg.streamRLP(rlpStream);
            session->sealAndSend(rlpStream);
        } else {

        }
    }
}

void NetController::broadcast(std::shared_ptr<core::Block> bMsg)
{
    CINFO << "Net broadcast block(" << bMsg->getNumber() << ")";


}

std::shared_ptr<core::Transaction> NetController::getTransactionFromCache()
{
    std::shared_ptr<core::Transaction> ret = nullptr;

    if (!this->transactionsQueue.empty()) {
        ret = this->transactionsQueue.front();
        this->transactionsQueue.pop();
    }

    return ret;
}

std::shared_ptr<core::Block> NetController::getBlockFromCache()
{
    std::shared_ptr<core::Block> ret = nullptr;

    if (!transactionsQueue.empty()) {
        ret = blocksQueue.front();
        blocksQueue.pop();
    }

    return ret;
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

} // end of namespace