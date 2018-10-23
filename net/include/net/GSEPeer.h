#pragma once

#include <net/PeerCapability.h>
#include <chain/Common.h>

namespace net {

class GSEPeer: public net::PeerCapability {
public:
    GSEPeer(std::weak_ptr<net::SessionFace> s, std::string const& name,
                  unsigned messageCount, unsigned offset, net::CapDesc const& cap);

    ~GSEPeer() override;

    static std::string name() { return "GSE"; }

    static u256 version() { return net::c_protocolVersion; }

    static unsigned messageCount() { return chain::PacketCount; }

private:

    /// Interpret an incoming message.
    bool interpretCapabilityPacket(unsigned id, core::RLP const& rlp) override;

};
}