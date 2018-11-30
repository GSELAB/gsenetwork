#include <net/Client.h>
#include <utils/Utils.h>
#include <config/Argument.h>

using namespace utils;
using namespace config;

namespace net {

bool Client::isSyncing() const
{
    return false;
}

void Client::doWork()
{
    int64_t current = currentTimestamp();
    if (current > m_lastTimestamp + 2000) {
        m_lastTimestamp = current;
        if (ARGs.m_syncFlag) {
            for (auto i : peerSessions()) {
                auto gsePeer = capabilityFromSession<GSEPeer>(*i.first);
                gsePeer->beat();
            }
        } else {
            stopWorking();
        }
    } else {
        if (isWorking()) {
            sleepMilliseconds(1000);
        }
    }
}

std::shared_ptr<PeerCapabilityFace> Client::newPeerCapability(
    DispatchFace* dispatcher, std::shared_ptr<SessionFace> const& _s, unsigned _idOffset,CapDesc const& _cap) {
    auto ret = HostCapability<GSEPeer>::newPeerCapability(dispatcher, _s, _idOffset, _cap);
    return ret;
}

void Client::start()
{
    startWorking();
    if (isWorking())
        return;
    doneWorking();
}


void Client::stop()
{
    if (isWorking()) {
        stopWorking();
    }
    terminate();
}
}