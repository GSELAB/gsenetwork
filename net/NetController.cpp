#include "net/NetController.h"
#include <net/All.h>
#include <core/Log.h>
#include <chain/Types.h>
#include <crypto/Valid.h>
#include <config/Argument.h>

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
        if (m_dispatcher) m_host->setDispatcher(m_dispatcher);
        auto hostCap = std::make_shared<Client>(*m_host, m_dispatcher);
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
    broadcast(*tMsg);
}

void NetController::broadcast(core::Transaction const& tMsg)
{
    core::RLPStream rlpStream;
    tMsg.streamRLP(rlpStream);
    bytes data = rlpStream.out();
    send(data, TransactionPacket);
    CINFO << "RPC broadcast tx success";
}

void NetController::broadcast(std::shared_ptr<core::Block> bMsg)
{
    core::RLPStream rlpStream;
    bMsg->streamRLP(rlpStream);
    bytes data = rlpStream.out();
    send(data, BlockPacket);
    CINFO << "Broadcast block success. number(" << bMsg->getNumber() << ")";
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
        } else {
            CINFO << "NetController::send not find session ,size:" << m_host->getSessionSize();
        }
    }
}

void NetController::broadcast(bi::tcp::endpoint const& from, Block& block)
{

}

void NetController::broadcast(bi::tcp::endpoint const& from, BlockPtr block)
{

}

void NetController::broadcast(bi::tcp::endpoint const& from, Transaction& tx)
{

}

void NetController::broadcast(bi::tcp::endpoint const& from, TransactionPtr tx)
{

}

void NetController::broadcast(bi::tcp::endpoint const& from, BlockState& bs)
{

}

void NetController::broadcast(bi::tcp::endpoint const& from, BlockStatePtr bsp)
{

}

void NetController::send(BlockState& bs)
{

}

void NetController::send(BlockStatePtr bsp)
{

}

} // end of namespace