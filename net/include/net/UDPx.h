#pragma once

#include <memory>
#include <deque>

#include <net/Common.h>
#include <core/RLP.h>
#include <core/Log.h>
#include <chain/Types.h>

namespace net {

class UDPDatagram {
public:
    UDPDatagram(boost::asio::ip::udp::endpoint const& ep) :
        locus(ep) {}

    UDPDatagram(boost::asio::ip::udp::endpoint const& ep, bytes in) :
        locus(ep), data(in) {}

    boost::asio::ip::udp::endpoint const& endpoint() const {
        return locus;
    }

    bytes data;
protected:
    boost::asio::ip::udp::endpoint locus;
    chain::ChainID chainID;  /* ? move to another place? */
};

// BytesDatagramFace could be signed
struct BytesDatagramFace : public UDPDatagram {
    static uint32_t futureFromEpoch(std::chrono::seconds _sec) {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>((std::chrono::system_clock::now() + _sec).time_since_epoch()).count());
    }

    static uint32_t secondsSinceEpoch() {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>((std::chrono::system_clock::now()).time_since_epoch()).count());
    }

    static Public authenticate(bytesConstRef sig, bytesConstRef rlp);

    // none static function
    BytesDatagramFace(boost::asio::ip::udp::endpoint const& ep) :
        UDPDatagram(ep) {}
    virtual ~BytesDatagramFace() = default;

    virtual h256 sign(Secret const& sec);
    virtual uint8_t packetType() const = 0;

    virtual void streamRLP(core::RLPStream &) const = 0;
    virtual void interpretRLP(bytesConstRef bs) = 0;
};

// interfce for udp socket
struct UDPSocketFace {
    virtual bool send(UDPDatagram const& msg) = 0;
    virtual void disconnect() = 0;
};

// interface for udp socket's owner
struct UDPSocketEvents {
    virtual ~UDPSocketEvents() = default;
    virtual void onDisconnected(UDPSocketFace *) {}
    virtual void onReceived(UDPSocketFace *, boost::asio::ip::udp::endpoint const& from, bytesConstRef pakcetData) = 0;
};

/**
 * Handler must implement UDPSocketEvents
 *
 */
template<typename Handler, unsigned MaxDatagramSize>
class UDPSocket : UDPSocketFace, public std::enable_shared_from_this<UDPSocket<Handler, MaxDatagramSize>> {
public:
    enum {
        maxDatagramSize = MaxDatagramSize,
    };

    static_assert((unsigned)maxDatagramSize < 65535u, "UDP datagrams cannot be larger than 65535 bytes");

    // specific endpoint
    UDPSocket(boost::asio::io_service& io, UDPSocketEvents& host, boost::asio::ip::udp::endpoint ep) :
        m_host(host), m_endpoint(ep), m_socket(io)
    {
        m_started.store(false);
        m_closed.store(true);
    }

    // listen to all ports
    UDPSocket(boost::asio::io_service& io, UDPSocketEvents& host, unsigned port) :
        m_host(host), m_endpoint(boost::asio::ip::udp::v4(), port), m_socket(io)
    {
        m_started.store(false);
        m_closed.store(true);
    }

    virtual ~UDPSocket() {
        disconnect();
    }

    void connect();
    bool send(UDPDatagram const& data);
    bool isOpen() {
        return !m_closed;
    }

    void disconnect() {
        disconnectWithError(boost::asio::error::connection_reset);
    }

protected:
    void doRead();
    void doWrite();
    void disconnectWithError(boost::system::error_code ec);

private:
    std::atomic<bool> m_started;
    std::atomic<bool> m_closed;

    UDPSocketEvents& m_host; // interface which owns this socket
    boost::asio::ip::udp::endpoint m_endpoint;

    Mutex x_sendQ;
    std::deque<UDPDatagram> m_sendQ; // queue for egress data
    std::array<byte, maxDatagramSize> m_recvData; // buffer for ingress data
    boost::asio::ip::udp::endpoint m_recvEndpoint; // data from received from
    boost::asio::ip::udp::socket m_socket; // boost asio udp socket

    Mutex x_socketError;
    boost::system::error_code m_socketError; // could be set from host or IO thread

    Logger m_logger{createLogger(VerbosityInfo, "udp")};
};


template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::connect()
{
    bool expect = false;

    if (!m_started.compare_exchange_strong(expect, true)) {
        return;
    }

    m_socket.open(boost::asio::ip::udp::v4());
    try {
        m_socket.bind(m_endpoint);
    } catch (...) {
        m_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), m_endpoint.port()));
    }

    // clear write queue so reconnect doesn't send stale messages
    Guard l(x_sendQ);
    m_sendQ.clear();

    m_closed = false;
    doRead();

}

template <typename Handler, unsigned MaxDatagramSize>
bool UDPSocket<Handler, MaxDatagramSize>::send(UDPDatagram const& data)
{
    if (m_closed) {
        LOG(m_logger) << "The udp socket has been closed.";
        return false;
    }

    Guard l(x_sendQ);
    m_sendQ.push_back(data);
    if (m_sendQ.size() == 1) {
        doWrite();
    }

    return true;
}

template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::doRead()
{
    if (m_closed) {
        LOG(m_logger) << "The udp socket has been closed.";
        return;
    }

    auto self(UDPSocket<Handler, MaxDatagramSize>::shared_from_this());
    m_socket.async_receive_from(boost::asio::buffer(m_recvData), m_recvEndpoint, [this, self](boost::system::error_code _ec, size_t _len) {
        if (m_closed) {
            return disconnectWithError(_ec);
        }

        if (_ec != boost::system::errc::success) {
            cnetlog << "Receiving UDP message failed. " << _ec.value() << " : " << _ec.message();
        }

        if (_len) {
            m_host.onReceived(this, m_recvEndpoint, bytesConstRef(m_recvData.data(), _len));
        }

        doRead();
    });
}

template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::doWrite()
{
    if (m_closed) {
        cnetlog << "The udp socket has been closed.";
        return;
    }

    const UDPDatagram& datagram = m_sendQ[0];
    auto self(UDPSocket<Handler, MaxDatagramSize>::shared_from_this());
    bi::udp::endpoint endpoint(datagram.endpoint());
    m_socket.async_send_to(boost::asio::buffer(datagram.data), endpoint, [this, self, endpoint](boost::system::error_code _ec, std::size_t)
    {
        if (m_closed)
            return disconnectWithError(_ec);

        if (_ec != boost::system::errc::success)
            cnetlog << "Failed delivering UDP message. " << _ec.value() << " : " << _ec.message();

        Guard l(x_sendQ);
        m_sendQ.pop_front();
        if (m_sendQ.empty())
            return;
        doWrite();
    });
}

template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::disconnectWithError(boost::system::error_code ec)
{
    if (!m_started && m_closed && !m_socket.is_open()) {
        return;
    }

    assert(ec);
    {
        // disconnect-operation following prior non-zero errors are ignored
        Guard l(x_socketError);
        if (m_socketError != boost::system::error_code())
            return;
        m_socketError = ec;
    }

    // prevent concurrent disconnect
    bool expected = true;
    if (!m_started.compare_exchange_strong(expected, false))
        return;

    // set m_closed to true to prevent undeliverable egress messages
    bool wasClosed = m_closed;
    m_closed = true;

    // close sockets
    boost::system::error_code tmpec;
    m_socket.shutdown(bi::udp::socket::shutdown_both, tmpec);
    m_socket.close();

    // socket never started if it never left stopped-state (pre-handshake)
    if (wasClosed)
        return;

    m_host.onDisconnected(this);
}
} // end namespace