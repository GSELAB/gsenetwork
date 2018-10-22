#include <net/GSEPeer.h>
#include <core/Log.h>

using namespace core;
using namespace chain;

namespace net {

GSEPeer::GSEPeer(std::weak_ptr<net::SessionFace> s, std::string const& name,
    unsigned messageCount, unsigned offset, net::CapDesc const& cap): net::PeerCapability(move(s), name, messageCount, offset)
{

}

GSEPeer::~GSEPeer()
{

}

bool GSEPeer::interpretCapabilityPacket(unsigned id, core::RLP const& rlp)
{
    CINFO << "GSEPeer::interpretCapabilityPacket id:" << id << "  - " << chain::pptToString((chain::ProtocolPacketType)id);
    try {
        switch (id) {
        case chain::StatusPacket:
            CINFO << "GSEPeer - Recv status packet.";

            return true;
        case chain::TransactionPacket:
            CINFO << "GSEPeer - Recv tx packet.";

            return true;
        case chain::TransactionsPacket:
            CINFO << "GSEPeer - Recv txs packet.";

            return true;
        case chain::BlockPacket:
            CINFO << "GSEPeer - Recv block packet.";
            return true;
        case chain::BlockBodiesPacket:
            CINFO << "GSEPeer - Recv blocks packet.";
            return true;
        case chain::NewBlockPacket:
            CINFO << "GSEPeer - Recv new block packet.";
            return true;
        default:
            CINFO << "GPeer - Unknown packet type - " << chain::pptToString((chain::ProtocolPacketType)id);
            return false;
        }

    } catch (...) {
        return false;
    }
}

}