/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file HostCapability.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Miscellanea required for the Host/Session classes.
 */

#pragma once

#include <memory>
#include <net/Peer.h>
#include <net/Common.h>
#include <net/Session.h>
#include <net/Host.h>

namespace net {

class DispatchFace;

class HostCapabilityFace
{
public:
    virtual ~HostCapabilityFace() = default;

    virtual std::string name() const = 0;
    virtual u256 version() const = 0;
    virtual unsigned messageCount() const = 0;

    virtual std::shared_ptr<PeerCapabilityFace> newPeerCapability(DispatchFace* dispatcher,
        std::shared_ptr<SessionFace> const& _s, unsigned _idOffset, CapDesc const& _cap) = 0;

    virtual void onStarting() = 0;
    virtual void onStopping() = 0;

    virtual DispatchFace* getDispatcher() const = 0;
};

template<class PeerCap>
class HostCapability: public HostCapabilityFace
{
public:
    explicit HostCapability(net::Host const& _host) : m_host(_host) {}

    std::string name() const override { return PeerCap::name(); }
    u256 version() const override { return PeerCap::version(); }
    unsigned messageCount() const override { return PeerCap::messageCount(); }

    std::shared_ptr<PeerCapabilityFace> newPeerCapability(DispatchFace* dispatcher,
        std::shared_ptr<SessionFace> const& _s, unsigned _idOffset, CapDesc const& _cap) override
    {
        auto p = std::make_shared<PeerCap>(dispatcher,
            std::weak_ptr<SessionFace>{_s}, name(), messageCount(), _idOffset, _cap);
        _s->registerCapability(_cap, p);
        return p;
    }
    void onStarting() override {}
    void onStopping() override {}

    DispatchFace* getDispatcher() const override { return nullptr; }

protected:
    CapDesc capDesc() const { return std::make_pair(name(), version()); }

    std::vector<std::pair<std::shared_ptr<SessionFace>, std::shared_ptr<Peer>>> peerSessions() const
    {
        return m_host.peerSessions(name(), version());
    }
    std::shared_ptr<SessionFace> peerSession(NodeID const& _id) const
    {
        return m_host.peerSession(_id);
    }

private:
    Host const& m_host;
};

} // endof namespace
