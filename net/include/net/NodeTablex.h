#pragma once

#include <algorithm>
#include <list>
#include <unordered_map>

#include <boost/integer/static_log2.hpp>

#include <net/Common.h>
#include <crypto/SHA3.h>
#include <chain/Types.h>
#include <net/UDPx.h>
#include <core/RLP.h>
#include <core/Log.h>

using namespace core;

namespace net {

struct NodeEntry : public Node {
    NodeEntry(NodeID const& nID, Public const& pubK, NodeIPEndpoint const &nIPEP);
    int const distance;
    chain::ChainID chainID;
    bool pending = true;
};

enum NodeTableEventType {
    NodeEntryAdded = 0,
    NodeEntryDropped,
};

class NodeTable;

class NodeTableEventHandler {
public:
    friend class NodeTable;

    virtual ~NodeTableEventHandler() = default;

    virtual void processEvent(NodeID const& nID, NodeTableEventType const& type) = 0;

protected:
    // Called by NodeTable on behalf of an implementation (Host) to process new events without blocking nodetable.
    void processEvents() {
        std::list<std::pair<NodeID, NodeTableEventType>> events;
        Guard l(x_events);
        if (!m_nodeEventHandler.size()) {
            return;
        }

        m_nodeEventHandler.unique();
        for (auto const& n: m_nodeEventHandler) {
            events.push_back(std::make_pair(n,m_events[n]));
        }

        m_nodeEventHandler.clear();
        m_events.clear();

        for (auto const& e: events) {
            processEvent(e.first, e.second);
        }
    }

    // Called by NodeTable to append event.
    virtual void appendEvent(NodeID nID, NodeTableEventType type) {
        Guard l(x_events);
        m_nodeEventHandler.push_back(nID);
        m_events[nID] = type;
    }

    Mutex x_events;
    std::list<NodeID> m_nodeEventHandler;
    std::unordered_map<NodeID, NodeTableEventType> m_events;

};

inline std::ostream& operator<<(std::ostream& _out, NodeTable const& _nodeTable);


/**
 *
 *
 *
 *
 *
 */
class NodeTable : UDPSocketEvents, public std::enable_shared_from_this<NodeTable> {

    // defaut private
    friend std::ostream& operator<<(std::ostream& _out, NodeTable const& _nodeTable);
    using NodeSocket = UDPSocket<NodeTable, 1024>;
    using TimePoint = std::chrono::steady_clock::time_point;	///< Steady time point.
    using NodeIdTimePoint = std::pair<NodeID, TimePoint>;


    /**
     * EvictionTimeout is used to record the timepoint of the evicted node
     * and the new node ID is used to replace it.
     */
    struct EvictionTimeout {
        NodeID newNodeID;
        TimePoint evictedTimePoint;
    };

public:
    enum NodeRelation {
        Unknown = 0,
        Known,
    };

    enum DiscoverType {
        Random = 0,
    };

    NodeTable(ba::io_service& _io, GKey const& key, NodeIPEndpoint const& _endpoint, bool _enabled = true);

    ~NodeTable();

    static int distance(NodeID const& x, NodeID const& y) {
        unsigned ret = 0;
        u256 d = sha3(x) ^ sha3(y);
        while (d >>= 1) ret++;
        return ret;
    }

    // handler for add and drop
    void setEventHandler(NodeTableEventHandler* handler) {
        m_nodeEventHandler.reset(handler);
    }

    // called by implementation which provided handler to process NodeEntryAdded/NodeEntryDropped events.
    // events are coalesced by type whereby old events are ignored.
    void processEvents();

    // add node
    std::shared_ptr<NodeEntry> addNode(Node const& node, NodeRelation relation = NodeRelation::Unknown);

    // return list of node ids active in node table
    std::list<NodeID> nodes() const;

    unsigned count() const {
        return m_nodes.size();
    }

    // Returns snapshot of table.
    std::list<NodeEntry> snapshot() const;

    // if node id in node table
    bool haveNode(NodeID const &id) {
        Guard l(x_nodes);
        return m_nodes.count(id) > 0;
    }

    // return the Node to the corresponding node id or the empty Node if that id is not found.
    Node node(NodeID const& id);

protected:
    static unsigned const s_addressByteSize = h256::size;					///< Size of address type in bytes.
    static unsigned const s_bits = 8 * s_addressByteSize;					///< Denoted by n in [Kademlia].
    static unsigned const s_bins = s_bits - 1;								///< Size of m_state (excludes root, which is us).
    static unsigned const s_maxSteps = boost::static_log2<s_bits>::value;	///< Max iterations of discovery. (discover)

    static unsigned const s_bucketSize = 16;			///< Denoted by k in [Kademlia]. Number of nodes stored in each bucket.
    static unsigned const s_alpha = 3;				///< Denoted by \alpha in [Kademlia]. Number of concurrent FindNode requests.

    // intervals
    // replace boost::posix_time; change constants to upper camelcase
    std::chrono::milliseconds const c_evictionCheckInterval = std::chrono::milliseconds(75);	///< Interval at which eviction timeouts are checked.
    std::chrono::milliseconds const c_reqTimeout = std::chrono::milliseconds(300);						///< How long to wait for requests (evict, find iterations).
    std::chrono::milliseconds const c_bucketRefresh = std::chrono::milliseconds(7200);							///< Refresh interval prevents bucket from becoming stale. [Kademlia]

    struct NodeBucket {
        unsigned distance;
        std::list<std::weak_ptr<NodeEntry>> nodes;
    };

    // used to ping endpoint
    void ping(NodeIPEndpoint to) const;

    // used ping known node. used by node table when refreshing buckets and as part of eviction process (see evict).
    void ping(NodeEntry *ne) const;

    // return center node entry which describes this node and used with dist() to calculate xor metric for node table nodes.
    NodeEntry center() const {
        return NodeEntry(m_node.id, m_node.publicKey(), m_node.endpoint);
    }

    // used by asynchronous operations to return NodeEntry which is active and managed by node table.
    std::shared_ptr<NodeEntry> nodeEntry(NodeID nID);

    // used to discovery nodes on network which are close to the given target.
    // sends s_alpha concurrent requests to nodes nearest to target, for nodes nearest to target, up to s_maxSteps rounds.
    void doDiscover(NodeID target, unsigned round = 0, std::shared_ptr<std::set<std::shared_ptr<NodeEntry>>> tried = std::shared_ptr<std::set<std::shared_ptr<NodeEntry>>>());

    // return nodes from node table which are closest to target.
    std::vector<std::shared_ptr<NodeEntry>> nearestNodeEntries(NodeID nID);

    // asynchronously drops _leastSeen node if it doesn't reply and adds _new node, otherwise _new node is thrown away.
    void evict(std::shared_ptr<NodeEntry> leastSeen, std::shared_ptr<NodeEntry> ne);

    // Called whenever activity is received from a node in order to maintain node table.
    void noteActiveNode(Public const& pubK, boost::asio::ip::udp::endpoint const& ep);

    // Used to drop node when timeout occurs or when evict() result is to keep previous node.
    void dropNode(std::shared_ptr<NodeEntry> ne);

    NodeBucket& bucket_UNSAFE(NodeEntry const* n);

    // general network events

    // called by m_socket when packet is received.
    void onReceived(UDPSocketFace*, boost::asio::ip::udp::endpoint const& from, bytesConstRef packet);

    // called by m_socket when socket is disconnected.
    void onDisconnected(UDPSocketFace*) {}

    // task

    // called by evict() to ensure eviction check is scheduled to run and terminates when no evictions remain. Asynchronous.
    void doCheckEvictions();

    // looks up a random node at @c_bucketRefresh interval.
    void doDiscovery();

protected:
    std::unique_ptr<NodeTableEventHandler> m_nodeEventHandler;
    Node m_node; // current node
    Secret m_secret;

    mutable Mutex x_nodes;
    std::unordered_map<NodeID, std::shared_ptr<NodeEntry>> m_nodes;

    mutable Mutex x_state;
    std::array<NodeBucket, s_bins> m_state; // < State of p2p node network.

    Mutex x_evictions; // LOCK x_evictions first if both x_nodes and x_evictions locks are required.
    std::unordered_map<NodeID, EvictionTimeout> m_evictions;  // Eviction timeouts.

    Mutex x_pubkDiscoverPings;										// < LOCK x_nodes first if both x_nodes and x_pubkDiscoverPings locks are required.
    std::unordered_map<boost::asio::ip::address, TimePoint> m_pubkDiscoverPings;	// < List of pending pings where node entry wasn't created due to unkown pubk.

    Mutex x_findNodeTimeout;
    std::list<NodeIdTimePoint> m_findNodeTimeout;					// < Timeouts for FindNode requests.

    std::shared_ptr<NodeSocket> m_socket;							// < Shared pointer for our UDPSocket; ASIO requires shared_ptr.
    NodeSocket* m_socketPointer;    // < Set to m_socket.get(). Socket is created in constructor and disconnected in destructor to ensure access to pointer is safe.

    Logger m_logger{createLogger(VerbosityDebug, "discovery")};

    DeadlineOps m_timers; ///< this should be the last member - it must be destroyed first
};

inline std::ostream& operator<<(std::ostream& _out, NodeTable const& _nodeTable)
{
    _out << _nodeTable.center().address() << "\t"
         << "0\t" << _nodeTable.center().endpoint.address() << ":"
         << _nodeTable.center().endpoint.udpPort() << std::endl;
    auto s = _nodeTable.snapshot();
    for (auto n: s)
        _out << n.address() << "\t" << n.distance << "\t" << n.endpoint.address() << ":"
             << n.endpoint.udpPort() << std::endl;
    return _out;
}

/* --------------------------------------------------------------------------------- */
enum DataType : uint8_t {
    PingNodeType = 1,
    PongType,
    FindNodeType,
    NeighboursType,
};

struct DiscoveryDatagram: public BytesDatagramFace
{
    /// Constructor used for sending.
    DiscoveryDatagram(bi::udp::endpoint const& _to): BytesDatagramFace(_to), ts(futureFromEpoch(std::chrono::seconds(60))) {}

    /// Constructor used for parsing inbound packets.
    DiscoveryDatagram(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): BytesDatagramFace(_from), sourceid(_fromid), echo(_echo) {}

    // These two are set for inbound packets only.
    NodeID sourceid; // sender public key (from signature)
    h256 echo;       // hash of encoded packet, for reply tracking

    // All discovery packets carry a timestamp, which must be greater
    // than the current local time. This prevents replay attacks.
    uint32_t ts = 0;
    bool isExpired() const { return secondsSinceEpoch() > ts; }

    /// Decodes UDP packets.
    static std::unique_ptr<DiscoveryDatagram> interpretUDP(bi::udp::endpoint const& _from, bytesConstRef _packet);
};

/**
 * Ping packet: Sent to check if node is alive.
 * PingNode is cached and regenerated after ts + t, where t is timeout.
 *
 * Ping is used to implement evict. When a new node is seen for
 * a given bucket which is full, the least-responsive node is pinged.
 * If the pinged node doesn't respond, then it is removed and the new
 * node is inserted.
 */
struct PingNode: DiscoveryDatagram
{
    using DiscoveryDatagram::DiscoveryDatagram;
    PingNode(NodeIPEndpoint const& _src, NodeIPEndpoint const& _dest): DiscoveryDatagram(_dest), source(_src), destination(_dest) {}
    PingNode(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): DiscoveryDatagram(_from, _fromid, _echo) {}

    static const uint8_t type = PingNodeType;
    uint8_t packetType() const { return type; }

    unsigned version = 0;
    NodeIPEndpoint source;
    NodeIPEndpoint destination;

    void streamRLP(core::RLPStream& _s) const
    {
        _s.appendList(4);
        _s << net::c_protocolVersion;
        source.streamRLP(_s);
        destination.streamRLP(_s);
        _s << ts;
    }
    void interpretRLP(bytesConstRef _bytes)
    {
        core::RLP r(_bytes, core::RLP::AllowNonCanon | core::RLP::ThrowOnFail);
        version = r[0].toInt<unsigned>();
        source.interpretRLP(r[1]);
        destination.interpretRLP(r[2]);
        ts = r[3].toInt<uint32_t>();
    }
};

/**
 * Pong packet: Sent in response to ping
 */
struct Pong: DiscoveryDatagram
{
    Pong(NodeIPEndpoint const& _dest): DiscoveryDatagram((bi::udp::endpoint)_dest), destination(_dest) {}
    Pong(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): DiscoveryDatagram(_from, _fromid, _echo) {}

    static const uint8_t type = PongType;
    uint8_t packetType() const { return type; }

    NodeIPEndpoint destination;

    void streamRLP(core::RLPStream& _s) const
    {
        _s.appendList(3);
        destination.streamRLP(_s);
        _s << echo;
        _s << ts;
    }
    void interpretRLP(bytesConstRef _bytes)
    {
        core::RLP r(_bytes, core::RLP::AllowNonCanon | core::RLP::ThrowOnFail);
        destination.interpretRLP(r[0]);
        echo = (h256)r[1];
        ts = r[2].toInt<uint32_t>();
    }
};

/**
 * FindNode Packet: Request k-nodes, closest to the target.
 * FindNode is cached and regenerated after ts + t, where t is timeout.
 * FindNode implicitly results in finding neighbours of a given node.
 *
 * RLP Encoded Items: 2
 * Minimum Encoded Size: 21 bytes
 * Maximum Encoded Size: 30 bytes
 *
 * target: NodeID of node. The responding node will send back nodes closest to the target.
 *
 */
struct FindNode: DiscoveryDatagram
{
    FindNode(bi::udp::endpoint _to, h512 _target): DiscoveryDatagram(_to), target(_target) {}
    FindNode(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): DiscoveryDatagram(_from, _fromid, _echo) {}

    static const uint8_t type = FindNodeType;
    uint8_t packetType() const { return type; }

    h512 target;

    void streamRLP(core::RLPStream& _s) const
    {
        _s.appendList(2); _s << target << ts;
    }
    void interpretRLP(bytesConstRef _bytes)
    {
        core::RLP r(_bytes, core::RLP::AllowNonCanon | core::RLP::ThrowOnFail);
        target = r[0].toHash<h512>();
        ts = r[1].toInt<uint32_t>();
    }
};

/**
 * Node Packet: One or more node packets are sent in response to FindNode.
 */
struct Neighbours: DiscoveryDatagram
{
    Neighbours(bi::udp::endpoint _to, std::vector<std::shared_ptr<NodeEntry>> const& _nearest, unsigned _offset = 0, unsigned _limit = 0): DiscoveryDatagram(_to)
    {
        auto limit = _limit ? std::min(_nearest.size(), (size_t)(_offset + _limit)) : _nearest.size();
        for (auto i = _offset; i < limit; i++)
            neighbours.push_back(Neighbour(*_nearest[i]));
    }
    Neighbours(bi::udp::endpoint const& _to): DiscoveryDatagram(_to) {}
    Neighbours(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): DiscoveryDatagram(_from, _fromid, _echo) {}

    struct Neighbour
    {
        Neighbour(Node const& _node): endpoint(_node.endpoint), node(_node.id) {}
        Neighbour(core::RLP const& _r): endpoint(_r) { node = h512(_r[3].toBytes()); }
        NodeIPEndpoint endpoint;
        NodeID node;
        void streamRLP(core::RLPStream& _s) const { _s.appendList(4); endpoint.streamRLP(_s, NodeIPEndpoint::StreamInline); _s << node; }
    };

    static const uint8_t type = NeighboursType;
    uint8_t packetType() const { return type; }

    std::vector<Neighbour> neighbours;

    void streamRLP(core::RLPStream& _s) const
    {
        _s.appendList(2);
        _s.appendList(neighbours.size());
        for (auto const& n: neighbours)
            n.streamRLP(_s);
        _s << ts;
    }
    void interpretRLP(bytesConstRef _bytes)
    {
        core::RLP r(_bytes, core::RLP::AllowNonCanon | core::RLP::ThrowOnFail);
        for (auto const& n: r[0])
            neighbours.emplace_back(n);
        ts = r[1].toInt<uint32_t>();
    }
};


} // end namespace