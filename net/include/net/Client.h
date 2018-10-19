#pragma once

#include <net/Common.h>
#include <net/HostCapability.h>
#include <core/Task.h>
#include <chain/GPeer.h>

namespace net {

class Client: public HostCapability<chain::GPeer>, core::Task{
public:
    Client(Host const& host);

    virtual ~Client();

    void reset();

    bool isSyncing() const;



private:

    // do block chain sync work
    virtual void doWork() override;

private:
    // member
    int64_t m_lastTimestamp = -1;

};
}

