#include <chain/GPeer.h>
#include <core/Log.h>

using namespace core;

namespace chain {

GPeer::GPeer(std::weak_ptr<net::SessionFace> s, std::string const& name,
    unsigned messageCount, unsigned offset, net::CapDesc const& cap): net::PeerCapability(move(s), name, messageCount, offset)
{

}

GPeer::~GPeer()
{

}

bool GPeer::interpretCapabilityPacket(unsigned id, core::RLP const& rlp)
{
    CINFO << "GPeer::interpretCapabilityPacket";
    try {
        switch (id) {

        default:
            return false;
        }

    } catch (...) {

    }

    return true;
}
} // namespace chain