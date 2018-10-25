#include <net/Client.h>
#include <utils/Utils.h>

using namespace utils;

namespace net {

Client::Client(Host const& host, DispatchFace* dispatcher):
    HostCapability<GSEPeer>(host), Task("SyncClient"), m_dispatcher(dispatcher)
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

std::shared_ptr<PeerCapabilityFace> Client::newPeerCapability(
    DispatchFace* dispatcher, std::shared_ptr<SessionFace> const& _s, unsigned _idOffset,CapDesc const& _cap) {
    auto ret = HostCapability<GSEPeer>::newPeerCapability(dispatcher, _s, _idOffset, _cap);

    /*
    auto cap = capabilityFromSession<chain::GPeer>(*_s, _cap.second);
    assert(cap);
    cap->init(
        protocolVersion(),
        m_networkId,
        m_chain.details().totalDifficulty,
        m_chain.currentHash(),
        m_chain.genesisHash(),
        m_hostData,
        m_peerObserver
    );
    */

    return ret;
}

} // namespace