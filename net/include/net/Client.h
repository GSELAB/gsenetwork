#pragma once

#include <net/Common.h>
#include <net/HostCapability.h>
#include <core/Task.h>
#include <net/GSEPeer.h>

namespace net {

class Client: public HostCapability<GSEPeer>, core::Task {
public:
    Client(Host const& host, DispatchFace* dispatcher);

    virtual ~Client();

    void reset();

    bool isSyncing() const;

    DispatchFace* getDispatcher() const override { return m_dispatcher; }

public:
    void start();

    void stop();

protected:
    std::shared_ptr<PeerCapabilityFace> newPeerCapability(
        DispatchFace* dispatcher, std::shared_ptr<SessionFace> const& _s, unsigned _idOffset,CapDesc const& _cap) override;

private:

    // do block chain sync work
    virtual void doWork() override;

private:
    // member
    DispatchFace* m_dispatcher;
    int64_t m_lastTimestamp = -1;

};
}

