#include <net/GSEPeer.h>
#include <core/Log.h>

using namespace core;
using namespace chain;

namespace net {

GSEPeer::GSEPeer(DispatchFace* dispatcher, std::weak_ptr<net::SessionFace> s, std::string const& name,
    unsigned messageCount, unsigned offset, net::CapDesc const& cap):
    m_dispatcher(dispatcher), net::PeerCapability(move(s), name, messageCount, offset)
{

}

GSEPeer::~GSEPeer()
{

}

bool GSEPeer::interpretCapabilityPacket(unsigned id, core::RLP const& rlp)
{
    PeerSessionInfo  peerInfo = session()->info();
    boost::system::error_code ec;
    bi::address address = bi::address::from_string(peerInfo.host, ec);
    bi::tcp::endpoint ep(bi::address(), peerInfo.port);
    if (!ec) ep.address(address);
    if (m_dispatcher)
        return m_dispatcher->processMsg(ep, id, rlp);

    return false;
}

}