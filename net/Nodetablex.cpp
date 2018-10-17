#include <net/NodeTablex.h>
#include <core/Log.h>

using namespace std;

namespace net {

namespace
{
// global thread-safe logger for static methods
BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(g_discoveryWarnLogger,
    boost::log::sources::severity_channel_logger_mt<>,
    (boost::log::keywords::severity = 0)(boost::log::keywords::channel = "discov"))
}  // namespace

inline bool operator==(std::weak_ptr<NodeEntry> const& _weak, std::shared_ptr<NodeEntry> const& _shared)
{
    return !_weak.owner_before(_shared) && !_shared.owner_before(_weak);
}

NodeEntry::NodeEntry(NodeID const& nID, Public const& pubK, NodeIPEndpoint const &nIPEP) :
    Node(pubK, nIPEP), distance(NodeTable::distance(nID, pubK)) {}

NodeTable::NodeTable(ba::io_service& _io, GKey const& key, NodeIPEndpoint const& _endpoint, bool _enabled):
    m_node(Node(key.getPublic(), _endpoint)),
    m_secret(key.getSecret()),
    m_socket(make_shared<NodeSocket>(_io, *reinterpret_cast<UDPSocketEvents*>(this), (bi::udp::endpoint)m_node.endpoint)),
    m_socketPointer(m_socket.get()),
    m_timers(_io)
{
    CINFO << "NodeTable::NodeTable constructor";
    for (unsigned i = 0; i < s_bins; i++)
        m_state[i].distance = i;

    if (!_enabled)
        return;

    try {
        m_socketPointer->connect();
        doDiscovery();
    } catch (std::exception const& e) {
        CWARN << "Exception connecting NodeTable socket: " << e.what();
        //cwarn << "Discovery disabled.";
    }
}

NodeTable::~NodeTable()
{
    m_socketPointer->disconnect();
    m_timers.stop();
}

void NodeTable::processEvents()
{
    if (m_nodeEventHandler)
        m_nodeEventHandler->processEvents();
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

std::list<NodeID> NodeTable::nodes() const
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
    CINFO << "round " << round;
    // note : only called by doDiscovery
    if (!m_socketPointer->isOpen()) return;
    if (round == s_maxSteps) {
        CINFO << "Terminating after " << round << "rounds";
        doDiscovery();
        return;
    } else if (!round && !tried) {
        tried = make_shared<set<shared_ptr<NodeEntry>>>();
    }

    auto nearest = nearestNodeEntries(target);
    list<shared_ptr<NodeEntry>> triedList;
    for (unsigned i = 0; i < nearest.size() && triedList.size() < s_alpha; i++) {
        if (!tried->count(nearest[i])) {
            auto r = nearest[i];
            triedList.push_back(r);
            FindNode p(r->endpoint, target);
            p.sign(m_secret);
            DEV_GUARDED(x_findNodeTimeout)
                m_findNodeTimeout.push_back(make_pair(r->id, chrono::steady_clock::now()));
            m_socketPointer->send(p);
        }
    }

    if (triedList.empty()) {
        CINFO << "Terminating after " << round << "rounds";
        doDiscovery();
        return;
    }

    while (!triedList.empty()) {
        tried->insert(triedList.front());
        triedList.pop_front();
    }

    m_timers.schedule(c_reqTimeout.count() * 2, [this, target, round, tried](boost::system::error_code const& ec) {
        if (ec)
            CWARN << "Discovery timer was probably cancelled: " << ec.value() << " " << ec.message();


        if (ec.value() == boost::asio::error::operation_aborted || m_timers.isStopped())
            return;

        // error::operation_aborted means that the timer was probably aborted.
        // It usually happens when "this" object is deallocated, in which case
        // subsequent call to doDiscover() would cause a crash. We can not rely on
        // m_timers.isStopped(), because "this" pointer was captured by the lambda,
        // and therefore, in case of deallocation m_timers object no longer exists.

        doDiscover(target, round + 1, tried);
    });
}

std::vector<std::shared_ptr<NodeEntry>> NodeTable::nearestNodeEntries(NodeID nID)
{
    // send s_alpha FindNode packets to nodes we know, closest to target
    static unsigned lastBin = s_bins - 1;
    unsigned head = distance(m_node.id, nID);
    unsigned tail = head == 0 ? lastBin : (head - 1) % s_bins;

    // if d is 0, then we roll look forward, if last, we reverse, else, spread from d
    map<unsigned, list<shared_ptr<NodeEntry>>> found;
    if (head > 1 && tail != lastBin) {
        while (head != tail && head < s_bins) {
            Guard l(x_state);
            for (auto const& n: m_state[head].nodes)
                if (auto p = n.lock())
                    found[distance(nID, p->id)].push_back(p);

            if (tail)
                for (auto const& n: m_state[tail].nodes)
                    if (auto p = n.lock())
                        found[distance(nID, p->id)].push_back(p);

            head++;
            if (tail)
                tail--;
        }
    } else if (head < 2) {
        while (head < s_bins) {
            Guard l(x_state);
            for (auto const& n: m_state[head].nodes)
                if (auto p = n.lock())
                    found[distance(nID, p->id)].push_back(p);
            head++;
        }
    } else {
        while (tail > 0) {
            Guard l(x_state);
            for (auto const& n: m_state[tail].nodes)
                if (auto p = n.lock())
                    found[distance(nID, p->id)].push_back(p);
            tail--;
        }
    }

    vector<shared_ptr<NodeEntry>> ret;
    for (auto& nodes: found)
        for (auto const& n: nodes.second)
            if (ret.size() < s_bucketSize && !!n->endpoint && n->endpoint.isAllowed())
                ret.push_back(n);
    return ret;
}

void NodeTable::ping(NodeIPEndpoint to) const
{
    NodeIPEndpoint src;
    DEV_GUARDED(x_nodes)
        src = m_node.endpoint;
    PingNode msg(src, to);
    msg.sign(m_secret);
    m_socketPointer->send(msg);
}

void NodeTable::ping(NodeEntry *ne) const
{
    if (ne) {
        ping(ne->endpoint);
    }
}

void NodeTable::evict(std::shared_ptr<NodeEntry> leastSeen, std::shared_ptr<NodeEntry> ne)
{
    if (!m_socketPointer->isOpen())
        return;

    unsigned evicts = 0;
    DEV_GUARDED(x_evictions)
    {
        EvictionTimeout evictTimeout{ne->id, chrono::steady_clock::now()};
        m_evictions.emplace(leastSeen->id, evictTimeout);
        evicts = m_evictions.size();
    }

    if (evicts == 1)
        doCheckEvictions();
    ping(leastSeen.get());
}

void NodeTable::noteActiveNode(Public const& pubK, boost::asio::ip::udp::endpoint const& ep)
{
    if (pubK == m_node.address() || !NodeIPEndpoint(ep.address(), ep.port(), ep.port()).isAllowed())
        return;

    shared_ptr<NodeEntry> newNode = nodeEntry(pubK);
    if (newNode && !newNode->pending) {
        LOG(m_logger) << "Noting active node: " << pubK << " " << ep.address().to_string()
                      << ":" << ep.port();
        newNode->endpoint.setAddress(ep.address());
        newNode->endpoint.setUdpPort(ep.port());

        shared_ptr<NodeEntry> nodeToEvict;
        {
            Guard l(x_state);
            // Find a bucket to put a node to
            NodeBucket& s = bucket_UNSAFE(newNode.get());
            auto& nodes = s.nodes;
            // check if the node is already in the bucket
            auto it = std::find(nodes.begin(), nodes.end(), newNode);
            if (it != nodes.end()) {
                // if it was in the bucket, move it to the last position
                nodes.splice(nodes.end(), nodes, it);
            } else {
                if (nodes.size() < s_bucketSize) {
                    // if it was not there, just add it as a most recently seen node
                    // (i.e. to the end of the list)
                    nodes.push_back(newNode);
                    if (m_nodeEventHandler)
                        m_nodeEventHandler->appendEvent(newNode->id, NodeEntryAdded);
                } else {
                    // if bucket is full, start eviction process for the least recently seen node
                    nodeToEvict = nodes.front().lock();
                    // It could have been replaced in addNode(), then weak_ptr is expired.
                    // If so, just add a new one instead of expired
                    if (!nodeToEvict)
                    {
                        nodes.pop_front();
                        nodes.push_back(newNode);
                        if (m_nodeEventHandler)
                            m_nodeEventHandler->appendEvent(newNode->id, NodeEntryAdded);
                    }
                }
            }
        }

        if (nodeToEvict)
            evict(nodeToEvict, newNode);
    }
}

void NodeTable::dropNode(std::shared_ptr<NodeEntry> ne)
{
    {
        Guard l(x_state);
        NodeBucket& s = bucket_UNSAFE(ne.get());
        s.nodes.remove_if(
            [ne](weak_ptr<NodeEntry> const& _bucketEntry) { return _bucketEntry == ne; });
    }

    // notify host
    LOG(m_logger) << "p2p.nodes.drop " << ne->id;
    if (m_nodeEventHandler)
        m_nodeEventHandler->appendEvent(ne->id, NodeEntryDropped);
}

NodeTable::NodeBucket& NodeTable::bucket_UNSAFE(NodeEntry const* n)
{
    return m_state[n->distance - 1];
}

void NodeTable::onReceived(UDPSocketFace*, boost::asio::ip::udp::endpoint const& from, bytesConstRef packet)
{
    try {
        unique_ptr<DiscoveryDatagram> datagram = DiscoveryDatagram::interpretUDP(from, packet);
        if (!datagram)
            return;

        if (datagram->isExpired()) {
            CINFO << "Invalid packet (timestamp in the past) from " << from.address().to_string() << ":" << from.port();
            return;
        }

        switch (datagram->packetType()) {
            case PingNodeType: {
                auto in = dynamic_cast<PingNode const&>(*datagram);
                in.source.setAddress(from.address());
                in.source.setUdpPort(from.port());
                addNode(Node(in.sourceid, in.source));

                Pong respone(in.source);
                respone.echo = in.echo;
                respone.sign(m_secret);
                m_socketPointer->send(respone);
                break;
            }
            case PongType: {
                auto in = dynamic_cast<Pong const&>(*datagram);
                // whenever a pong is received, check if it's in m_evictions
                bool found = false;
                NodeID leastSeenID;
                EvictionTimeout evictionEntry;
                DEV_GUARDED(x_evictions)
                {
                    auto e = m_evictions.find(in.sourceid);
                    if (e != m_evictions.end()) {
                        if (e->second.evictedTimePoint > std::chrono::steady_clock::now()) {
                            found = true;
                            leastSeenID = e->first;
                            evictionEntry = e->second;
                            m_evictions.erase(e);
                        }
                    }
                }

                if (found) {
                    if (auto n = nodeEntry(evictionEntry.newNodeID))
                        dropNode(n);
                    if (auto n = nodeEntry(leastSeenID))
                        n->pending = false;
                } else {
                    // if not, check if it's known/pending or a pubk discovery ping
                    if (auto n = nodeEntry(in.sourceid)) {
                        n->pending = false;
                    } else {
                        DEV_GUARDED(x_pubkDiscoverPings)
                        {
                            if (!m_pubkDiscoverPings.count(from.address()))
                                return; // unsolicited pong; don't note node as active
                            m_pubkDiscoverPings.erase(from.address());
                        }
                        if (!haveNode(in.sourceid))
                            addNode(Node(in.sourceid, NodeIPEndpoint(from.address(), from.port(), from.port())));
                    }
                }

                // update our endpoint address and UDP port
                DEV_GUARDED(x_nodes)
                {
                    if ((!m_node.endpoint || !m_node.endpoint.isAllowed()) &&
                        isPublicAddress(in.destination.address()))
                        m_node.endpoint.setAddress(in.destination.address());
                    m_node.endpoint.setUdpPort(in.destination.udpPort());
                }

                LOG(m_logger) << "PONG from " << in.sourceid << " " << from;
                break;
            }
            case FindNodeType: {
                auto in = dynamic_cast<FindNode const&>(*datagram);
                vector<shared_ptr<NodeEntry>> nearest = nearestNodeEntries(in.target);
                static unsigned const nlimit = (m_socketPointer->maxDatagramSize - 109) / 90; // ?? maybe modified due to the size
                for (unsigned offset = 0; offset < nearest.size(); offset += nlimit) {
                    Neighbours out(from, nearest, offset, nlimit);
                    out.sign(m_secret);
                    if (out.data.size() > 1024) {
                        cnetlog << "Sending truncated datagram, size: " << out.data.size();
                    }

                    m_socketPointer->send(out);
                }
                break;
            }
            case NeighboursType: {
                auto in = dynamic_cast< Neighbours const&>(*datagram);
                bool excepted = false;
                auto now = chrono::steady_clock::now();
                DEV_GUARDED(x_findNodeTimeout)
                m_findNodeTimeout.remove_if([&](NodeIdTimePoint const& t)
                {
                    if (t.first == in.sourceid && now - t.second < c_reqTimeout)
                        excepted = true;
                    else if (t.first == in.sourceid)
                        return true;
                    return false;
                });

                if (!excepted) {
                    cnetdetails << "Dropping unsolicited neighbours packet from "
                                << from.address();
                    break;
                }

                for (auto n: in.neighbours)
                    addNode(Node(n.nID, n.endpoint));
                break;
            }
        }

        noteActiveNode(datagram->sourceid, from);
    } catch(std::exception const&e) {
        CINFO << "Exception processing message from " << from.address().to_string() << ":" << from.port() << ": " << e.what();
    } catch(...) {
        CINFO << "Exception processing message from " << from.address().to_string() << ":" << from.port();
    }
}

void NodeTable::doCheckEvictions()
{
    m_timers.schedule(c_evictionCheckInterval.count(), [this](boost::system::error_code const& _ec)
    {
        if (_ec) {
            // we can't use m_logger here, because captured this might be already destroyed
            //clog(VerbosityDebug, "discov")
            //    << "Check Evictions timer was probably cancelled: " << _ec.value() << " "
            //    << _ec.message();
        }

        if (_ec.value() == boost::asio::error::operation_aborted || m_timers.isStopped())
            return;

        bool evictionsRemain = false;
        list<shared_ptr<NodeEntry>> drop;
        {
            Guard le(x_evictions);
            Guard ln(x_nodes);
            for (auto& e: m_evictions)
                if (chrono::steady_clock::now() - e.second.evictedTimePoint > c_reqTimeout)
                    if (m_nodes.count(e.second.newNodeID))
                        drop.push_back(m_nodes[e.second.newNodeID]);
            evictionsRemain = (m_evictions.size() - drop.size() > 0);
        }

        drop.unique();
        for (auto n: drop)
            dropNode(n);

        if (evictionsRemain)
            doCheckEvictions();
    });
}

void NodeTable::doDiscovery()
{
    m_timers.schedule(c_bucketRefresh.count(), [this](boost::system::error_code const& _ec)
    {
        if (_ec) {
            // we can't use m_logger here, because captured this might be already destroyed
            CERROR << "Discovery timer was probably cancelled: " << _ec.value() << " "  << _ec.message();
        }

        if (_ec.value() == boost::asio::error::operation_aborted || m_timers.isStopped())
            return;

        CINFO << "performing random discovery";
        NodeID randNodeId;


        crypto::Nonce::get().ref().copyTo(randNodeId.ref().cropped(0, h256::size));
        crypto::Nonce::get().ref().copyTo(randNodeId.ref().cropped(h256::size, h256::size));
        doDiscover(randNodeId);
    });
}

unique_ptr<DiscoveryDatagram> DiscoveryDatagram::interpretUDP(boost::asio::ip::udp::endpoint const& from, bytesConstRef packet)
{
    unique_ptr<DiscoveryDatagram> decoded;
    // h256 + Signature + type + chainID + RLP ()
    CINFO << "Hunter:------------------- interpretUDP";
    if (packet.size() < h256::size + Signature::size + 1 + 3 + sizeof(chain::ChainID)) {
        LOG(g_discoveryWarnLogger::get()) << "Invalid packet (too small) from "
                                          << from.address().to_string() << ":" << from.port();
        return decoded;
    }

    bytesConstRef hashedBytes(packet.cropped(h256::size, packet.size() - h256::size));
    bytesConstRef signedBytes(hashedBytes.cropped(Signature::size, hashedBytes.size() - Signature::size));
    bytesConstRef signatureBytes(packet.cropped(h256::size, Signature::size));
    bytesConstRef bodyBytes(packet.cropped(h256::size + Signature::size + 1));

    h256 echo(sha3(hashedBytes));
    if (!packet.cropped(0, h256::size).contentsEqual(echo.asBytes())) {
        LOG(g_discoveryWarnLogger::get()) << "Invalid packet (bad hash) from "
                                          << from.address().to_string() << ":" << from.port();
        return decoded;
    }

    Public sourceid(crypto::recover(*(Signature const*)signatureBytes.data(), sha3(signedBytes)));
    if (!sourceid) {
        LOG(g_discoveryWarnLogger::get()) << "Invalid packet (bad signature) from "
                                          << from.address().to_string() << ":" << from.port();
        return decoded;
    }

    switch (signedBytes[0]) {
        case PingNodeType: {
            decoded.reset(new PingNode(from, sourceid, echo));
            break;
        }
        case PongType: {
            decoded.reset(new Pong(from, sourceid, echo));
            break;
        }
        case FindNodeType: {
            decoded.reset(new FindNode(from, sourceid, echo));
            break;
        }
        case NeighboursType: {
            decoded.reset(new Neighbours(from, sourceid, echo));
            break;
        }
        default: {
            LOG(g_discoveryWarnLogger::get()) << "Invalid packet (unknown packet type) from "
                                              << from.address().to_string() << ":" << from.port();
            return decoded;
        }
    }

    decoded->interpretRLP(bodyBytes);
    return decoded;
}


} // end namespace






