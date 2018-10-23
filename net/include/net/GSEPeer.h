#pragma once

#include <net/PeerCapability.h>
#include <chain/Common.h>
#include <net/BytesPacket.h>

namespace net {

class DispatchFace {
public:
    virtual ~DispatchFace() {}

    virtual void processMsg(bi::tcp::endpoint const& from, BytesPacket const& msg) = 0;

    virtual bool processMsg(bi::tcp::endpoint const& from, unsigned type, RLP const& rlp) = 0;

    virtual bool processMsg(bi::tcp::endpoint const& from, unsigned type, bytes const& data) = 0;
};

class GSEPeer: public net::PeerCapability {
public:
    GSEPeer(DispatchFace* dispatcher, std::weak_ptr<net::SessionFace> s, std::string const& name,
                  unsigned messageCount, unsigned offset, net::CapDesc const& cap);

    ~GSEPeer() override;

    static std::string name() { return "GSE"; }

    static u256 version() { return net::c_protocolVersion; }

    static unsigned messageCount() { return chain::PacketCount; }

private:

    /// Interpret an incoming message.
    bool interpretCapabilityPacket(unsigned id, core::RLP const& rlp) override;

private:
    DispatchFace* m_dispatcher;
};
}