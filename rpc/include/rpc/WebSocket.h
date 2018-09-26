#pragma once

#include <boost/asio.hpp>

#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

namespace rpc {

namespace ba = boost::asio;
namespace bi = boost::asio::ip;

class WebSocketEventHandlerFace {
public:
    virtual ~WebSocketEventHandlerFace() {}

};

struct AsioStubConfig: public websocketpp::config::asio {
    // pull default settings from our core config
    typedef websocketpp::config::asio core;
    typedef core::concurrency_type concurrency_type;
    typedef core::request_type request_type;
    typedef core::response_type response_type;
    typedef core::message_type message_type;
    typedef core::con_msg_manager_type con_msg_manager_type;
    typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;
    typedef core::alog_type alog_type;
    typedef core::elog_type elog_type;
    typedef core::rng_type rng_type;
    typedef core::endpoint_base endpoint_base;

    static bool const enable_multithreading = true;

    struct transport_config : public core::transport_config {
        typedef core::concurrency_type concurrency_type;
        typedef core::elog_type elog_type;
        typedef core::alog_type alog_type;
        typedef core::request_type request_type;
        typedef core::response_type response_type;
        static bool const enable_multithreading = true;
    };

    typedef websocketpp::transport::asio::endpoint<transport_config>        transport_type;

    static const websocketpp::log::level elog_level =        websocketpp::log::elevel::all;
    static const websocketpp::log::level alog_level =        websocketpp::log::alevel::all;
};

//typedef websocketpp::server<websocketpp::config::core> RpcServer;
typedef websocketpp::server<AsioStubConfig> RpcServer;
typedef websocketpp::connection_hdl ConnectHDL;
typedef RpcServer::message_ptr MessagePtr;

class WebSocket {
public:
    WebSocket(unsigned short listenPort);

    ~WebSocket();

    bool socketInit();

    bool shutdown();

    void onMessage(ConnectHDL hdl, MessagePtr msg);

    void onHttp(RpcServer* server, ConnectHDL hdl);

    void send();

private:
    unsigned short m_listenPort;
    RpcServer m_rpcServer;
    // boost::optional<bi::tcp::endpoint> m_httpListenEndpoint;
};
}