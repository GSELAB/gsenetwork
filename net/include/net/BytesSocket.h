#pragma once

#include <chain/Types.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace net {

class BytesSocket : public std::enable_shared_from_this<BytesSocket> {
public:
    BytesSocket(boost::asio::io_service& ioService, chain::ChainID id);
    ~BytesSocket();

    bool isConnected() const;
    void close();
    boost::asio::ip::tcp::endpoint remoteEndpoint();
    boost::asio::ip::tcp::socket& ref();

    void setChainId(chain::ChainID id);
    chain::ChainID getChainId() const;

protected:
    chain::ChainID m_chainId;
    boost::asio::ip::tcp::socket m_socket;
};

} // end namespace