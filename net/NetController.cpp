#include "net/NetController.h"
#include <net/All.h>
#include <core/Log.h>
#include <chain/Types.h>
#include <crypto/Valid.h>
#include <config/Argument.h>
#include <net/Client.h>

using namespace config;
using namespace chain;

namespace net {

NetController::NetController(crypto::GKey const& key, DispatchFace* dispatcher):
    m_key(key), m_inited(false), m_dispatcher(dispatcher),
    m_networkConfig(ARGs.m_local.m_address.to_string(), ARGs.m_local.m_tcpPort, false),
    m_host(new Host("GSE V1.0", m_networkConfig))
{

}

NetController::NetController(crypto::GKey const& key, DispatchFace* dispatcher, config::NetConfig const& netConfig): NetController(key, dispatcher)
{
}

NetController::~NetController()
{
    CINFO << "NetController::~NetController";
}

void NetController::init()
{
    if (!m_inited) {
        if (m_dispatcher) m_host->addDispatcher(m_dispatcher);
        auto hostCap = std::make_shared<Client>(*m_host);
        m_host->registerCapability(hostCap);
        m_host->start();
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
    core::RLPStream rlpStream;
    tMsg.streamRLP(rlpStream);
    bytes data = rlpStream.out();
    CINFO << "NetController::broadcast tx";
    send(data, TransactionPacket);
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
            CINFO << "NetController::send data, find session";
            core::RLPStream rlpStream;
            prepare(rlpStream, packetType, 1).appendRaw(data);
            session->sealAndSend(rlpStream);
        } else {
            CINFO << "NetController::send not find session ,size:" << m_host->getSessionSize();
        }
    }
}

} // end of namespace