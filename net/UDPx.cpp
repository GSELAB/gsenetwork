#include <net/UDPx.h>
#include <crypto/Common.h>
#include <crypto/SHA3.h>

using namespace net;

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

/* --------------------------------------------------------------- */
h256 BytesDatagramFace::sign(Secret const& sec)
{
    h256 sighash;

    return sighash;
}

Public BytesDatagramFace::authenticate(bytesConstRef sig, bytesConstRef rlp)
{
    Signature const& signature = *(Signature const*)sig.data();
    return crypto::recover(signature, sha3(rlp));
}