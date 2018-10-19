#pragma once

#include <string>
#include <mutex>
#include <array>
#include <memory>
#include <utility>

#include <core/RLP.h>
#include <core/Guards.h>
#include <chain/Common.h>
#include <net/PeerCapability.h>

namespace chain {

class GPeer: public net::PeerCapability {
public:
    GPeer(std::weak_ptr<net::SessionFace> s, std::string const& name,
                  unsigned messageCount, unsigned offset, net::CapDesc const& cap);

    ~GPeer() override;

    static std::string name() { return "GSE"; }

    static u256 version() { return net::c_protocolVersion; }

    static unsigned messageCount() { return PacketCount; }

private:

    /// Interpret an incoming message.
    bool interpretCapabilityPacket(unsigned id, core::RLP const& rlp) override;

};
}