#include <net/NodeTablex.h>
#include <core/Log.h>

using namespace net;

inline bool operator==(std::weak_ptr<NodeEntry> const& _weak, std::shared_ptr<NodeEntry> const& _shared)
{
    return !_weak.owner_before(_shared) && !_shared.owner_before(_weak);
}

NodeEntry::NodeEntry(NodeID const& nID, Public const& pubK, NodeIPEndpoint const &nIPEP) : Node(pubK, nIPEP)
{
    distance = NodeTable::distance(nID, pubK);
}

NodeTable::NodeTable(boost::asio::io_service& io, KeyPair const& alias, NodeIPEndpoint const& endpoint, bool enable = true) :
    m_node(Node(alias.pub()), endpoint),
    m_secret(alias.secret()),
    m_socket(make_shared<NodeSocket>(io, *reinterpret_cast<UDPSocketEvents*>(this), (boost::asio::ip::udp::endpoint)m_node.endpoint)),
    m_socketPointer(m_socket.get()),
    m_timers(io)
{
    for (unsigned i = 0; i < s_bins; i++) {
        m_state[i].distance = i;
    }

    if (!enable) {
        cwarn << "Not enable ";
        return;
    }

    try {
        m_socketPointer->connect();
        doDiscovery();
    } catch (std::exception const& e) {
        cwarn << "Exception connecting NodeTable socket: " << _e.what();
        cwarn << "Discovery disabled.";
    }
}

Nodetable::~NodeTbale()
{
    m_socketPointer->disconnect();
    m_timers.stop();
}

shared_ptr<NodeEntry> NodeTable::addNode(Node const& node, NodeRelation relation)
{
    if (relation == Known) {
        auto ret = make_shared<NodeEntry>(m_node.id, node.id, node.endpoint);
        ret->pending = false;
        DEV_GUARDED(x_nodes)
            m_nodes[node.id] = ret;

        noteActiveNode(node.id, node.endpoint);
        return ret;
    }

    if (!node.endpoint) return shared_ptr<NodeEntry>();
    if (!node.id) {
        // ping address to recover nodeid if nodeid is empty
        DEV_GUARDED(x_nodes)
        {
            LOG(m_logger) << "Sending public key discovery Ping to "
                          << (bi::udp::endpoint)node.endpoint
                          << " (Advertising: " << (bi::udp::endpoint)m_node.endpoint << ")";
        }
        DEV_GUARDED(x_pubkDiscoverPings)
        {
            m_pubkDiscoverPings[node.endpoint.address()] = std::chrono::steady_clock::now();
        }
        ping(node.endpoint);
        return shared_ptr<NodeEntry>();
    }

    DEV_GUARDED(x_nodes)
        if (m_nodes.count(node.id))
            return m_nodes[node.id];

    auto ret = make_shared<NodeEntry>(m_node.id, node.id, node.endpoint);
    DEV_GUARDED(x_nodes)
    {
        m_nodes[node.id] = ret;
    }
    LOG(m_logger) << "addNode pending for " << node.endpoint;
    ping(node.endpoint);
    return ret;
}

std::list<NodeID> Nodetable::nodes() const
{
    std::list<NodeID> nodes;
    DEV_GUARDED(x_nodes)
        for (auto& i: m_nodes)
            nodes.push_back(i.second->id);

    return nodes;
}

std::list<NodeEntry> NodeTable::snapshot() const
{
    std::list<NodeEntry> ret;
    DEV_GUARDED(x_state)
        for (auto const& s: m_state)
            for (auto const& np: s.nodes)
                if (auto n = np.lock())
                    ret.push_back(*n);
    return ret;
}

Node NodeTable::node(NodeID const& id)
{
    Guard l(x_nodes);
    if (m_nodes.count(id)) {
        auto entry = m_nodes[id];
        return Node(id, entry->endpoint, entry->peerType);
    }

    return UnspecifiedNode;
}

std::shared_ptr<NodeEntry> NodeTable::nodeEntry(NodeID nID)
{
    Guard l(x_nodes);
    return m_nodes.count(nID) ? m_nodes[nID] : shared_ptr<NodeEntry>();
}

void NodeTable::doDiscover(NodeID target, unsigned round, std::shared_ptr<std::set<std::shared_ptr<NodeEntry>>> tried)
{
    // note : only called by doDiscovery

}

std::vector<std::shared_ptr<NodeEntry>> NodeTable::nearestNodeEntries(NodeID nID)
{


    std::vector<std::shared_ptr<NodeEntry>> ret;
    return ret;
}

void NodeTable::ping(NodeIPEndpoint to) const
{
    NodeIPEndpoint src;
    DEV_GUARDED(x_nodes)
        src = m_node.endpoint;
    PingNode msg(src, to);
    p.sign(m_secret);
    m_socketPointer->send(msg);
}

void NodeTable::ping(NodeEntry *ne) const
{
    if (ne)
        ping(ne->endpoint);
}













