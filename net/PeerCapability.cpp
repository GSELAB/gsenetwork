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

#include <net/PeerCapability.h>

#include <core/Log.h>
#include <net/Session.h>
#include <net/Host.h>

using namespace core;

namespace net {

PeerCapability::PeerCapability(std::weak_ptr<SessionFace> _s, std::string const& _name,
    unsigned _messageCount, unsigned _idOffset)
  : m_session(move(_s)), m_name(_name), m_messageCount(_messageCount), m_idOffset(_idOffset)
{
    cnetdetails << "New session for capability " << m_name << "; idOffset: " << m_idOffset;
}

void PeerCapability::disable(std::string const& _problem)
{
    cnetdetails << "DISABLE: Disabling capability '" << m_name << "'. Reason: " << _problem;
    m_enabled = false;
}

core::RLPStream& PeerCapability::prep(core::RLPStream& _s, unsigned _id, unsigned _args)
{
    return _s.appendRaw(bytes(1, _id + m_idOffset)).appendList(_args);
}

void PeerCapability::sealAndSend(core::RLPStream& _s)
{
    std::shared_ptr<SessionFace> session = m_session.lock();
    if (session)
        session->sealAndSend(_s);
}

void PeerCapability::addRating(int _r)
{
    std::shared_ptr<SessionFace> session = m_session.lock();
    if (session)
        session->addRating(_r);
}
} // namespace net