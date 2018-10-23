#pragma once

#include <net/Common.h>
#include <net/HostCapability.h>
#include <core/Task.h>
#include <net/GSEPeer.h>

namespace net {

class Client: public HostCapability<GSEPeer>, core::Task{
public:
    Client(Host const& host);

    virtual ~Client();

    void reset();

    bool isSyncing() const;

protected:
    std::shared_ptr<PeerCapabilityFace> newPeerCapability(std::shared_ptr<SessionFace> const& _s, unsigned _idOffset,CapDesc const& _cap) override;

private:

    // do block chain sync work
    virtual void doWork() override;

private:
    // member
    int64_t m_lastTimestamp = -1;

};
}

