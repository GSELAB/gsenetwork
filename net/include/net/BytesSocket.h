#pragma once

#include <chain/Types.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace net {

class BytesSocket : public std::enable_shared_from_this<BytesSocket> {
public:
    BytesSocket(boost::asio::io_service& ioService);
    ~BytesSocket();

    bool isConnected() const;
    void close();
    boost::asio::ip::tcp::endpoint remoteEndpoint();
    boost::asio::ip::tcp::socket& ref();

protected:
    boost::asio::ip::tcp::socket m_socket;
};

} // end namespace