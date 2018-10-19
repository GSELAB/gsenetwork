#include <net/Client.h>
#include <utils/Utils.h>

using namespace utils;

namespace net {

Client::Client(Host const& host):
    HostCapability<chain::GPeer>(host), Task("SyncClient")
{

}

Client::~Client()
{
    terminate();
}

void Client::reset()
{

}

bool Client::isSyncing() const
{
    return false;
}

// do block chain sync work
void Client::doWork()
{
    {

    }

    int64_t current = currentTimestamp();
    if (current > m_lastTimestamp + 1) {
        // do check height
        m_lastTimestamp = current;
    } else {
        sleepMilliseconds(200);
    }
}

} // namespace