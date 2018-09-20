#pragma once

#include <boost/asio.hpp>

#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>

namespace rpc {

namespace ba = boost::asio;
namespace bi = boost::asio::ip;

typedef websocketpp::server<websocketpp::config::core> RpcServer;
typedef websocketpp::connection_hdl ConnectHDL;
typedef RpcServer::message_ptr MessagePtr;

class WebSocketEventHandlerFace {
public:
    virtual ~WebSocketEventHandlerFace() {}

};

class WebSocket {
public:
    WebSocket();

    ~WebSocket();

    bool start();

    bool shutdown();

    void onMessage(ConnectHDL hdl, MessagePtr msg);

private:
    RpcServer m_rpcServer;
};
}