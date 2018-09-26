
#include <rpc/WebSocket.h>

namespace rpc {

//using websocketpp::lib::bind;

void on_message(RpcServer* server, ConnectHDL hdl, MessagePtr msg)
{
    //socket
}

void on_http(RpcServer* server, ConnectHDL hdl)
{

}

void on_socket_init(ConnectHDL hdl, boost::asio::ip::tcp::socket& s)
{

}

WebSocket::WebSocket(unsigned short listenPort): m_listenPort(listenPort)
{

}

WebSocket::~WebSocket()
{

}

bool WebSocket::socketInit()
{
    try {
        m_rpcServer.clear_access_channels(websocketpp::log::alevel::all);
        m_rpcServer.clear_error_channels(websocketpp::log::alevel::all);

        m_rpcServer.init_asio();
        m_rpcServer.set_reuse_addr(true);

        m_rpcServer.set_message_handler(websocketpp::lib::bind(&on_message, &m_rpcServer, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_rpcServer.set_socket_init_handler(websocketpp::lib::bind(&on_socket_init, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_rpcServer.set_http_handler(websocketpp::lib::bind(&on_http, &m_rpcServer, websocketpp::lib::placeholders::_1));

        m_rpcServer.set_listen_backlog(8190);
        m_rpcServer.listen(m_listenPort);
        m_rpcServer.start_accept();

        //??? should start multi threads
        if (1) {
            m_rpcServer.run();
        } else {
            /*
            typedef websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_ptr;

            std::vector<thread_ptr> ts;
            for (size_t i = 0; i < num_threads; i++) {
                ts.push_back(websocketpp::lib::make_shared<websocketpp::lib::thread>(&server::run, &m_rpcServer));
            }

            for (size_t i = 0; i < num_threads; i++) {
                ts[i]->join();
            }
            */
        }
    } catch(websocketpp::exception const & e) {
        throw e;
    }

    return true;
}

bool WebSocket::shutdown()
{
    return true;
}

void WebSocket::onMessage(ConnectHDL hdl, MessagePtr msg)
{

}

} // namespace rpc

