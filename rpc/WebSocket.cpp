#include <string>
#include <rpc/WebSocket.h>
#include <core/Log.h>

using namespace std;

namespace rpc {

namespace url_handler {

}

#define CALL()

void WebSocket::registerUrlHandlers()
{
    // get
    addHandler("get_version", [&](std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "get_version";
    });
    // addHandler("getinfo", );

    // put

    addHandler("create_transaction", [&](std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "create_transaction";
    });

    addHandler("push_transaction", [&](std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "push_transaction";
    });
}

void on_message(RpcServer* server, ConnectHDL hdl, MessagePtr msg)
{
    server->send(hdl, msg->get_payload(), msg->get_opcode());
}

WebSocket::WebSocket(unsigned short listenPort): m_listenPort(listenPort)
{
    // CINFO << "WebSocket::WebSocket";
}

WebSocket::~WebSocket()
{
    //CINFO << "Stop rpc service";
}

bool WebSocket::init()
{
    registerUrlHandlers();

    try {
        m_rpcServer.clear_access_channels(websocketpp::log::alevel::all);
        m_rpcServer.clear_error_channels(websocketpp::log::alevel::all);

        m_rpcServer.init_asio();
        m_rpcServer.set_reuse_addr(true);

        // m_rpcServer.set_message_handler(websocketpp::lib::bind(&on_message, &m_rpcServer, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        //// m_rpcServer.set_socket_init_handler(websocketpp::lib::bind(&on_socket_init, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        // m_rpcServer.set_http_handler(websocketpp::lib::bind(&on_http, &m_rpcServer, websocketpp::lib::placeholders::_1));

        m_rpcServer.set_message_handler(websocketpp::lib::bind(&on_message, &m_rpcServer, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_rpcServer.set_http_handler([&](ConnectHDL hdl) {
            onHttp(&m_rpcServer, hdl);
        });

        // m_rpcServer.set_listen_backlog(8190);
        m_rpcServer.listen(m_listenPort);
        m_rpcServer.start_accept();

        m_initialSuccess = true;
    } catch(websocketpp::exception const& e) {
        CERROR << "RPC server init failed, " << e.what();
        throw e;
    }

    return true;
}

void WebSocket::startService()
{
    try {
        if (m_initialSuccess) {
            //??? should start multi threads
            if (1) {
                m_rpcServer.run();
            } else {

            }
        }

    } catch(websocketpp::exception const& e) {
        CERROR << "RPC server run failed, " << e.what();
    }
}

bool WebSocket::shutdown()
{
    return true;
}

void WebSocket::onMessage(RpcServer* server, ConnectHDL hdl, MessagePtr msg)
{
    server->send(hdl, msg->get_payload(), msg->get_opcode());
}

void WebSocket::onHttp(RpcServer* server, ConnectHDL hdl)
{
    try {
        RpcServer::connection_ptr con = server->get_con_from_hdl(hdl);
        bool secure = con->get_uri()->get_secure();
        bi::tcp::endpoint const& ep = con->get_socket().lowest_layer().local_endpoint();

        Request const& req = con->get_request();

        string const body = con->get_request_body();
        string const resource = con->get_resource();

        CINFO << ep << ":" << "resource-" << resource;



    } catch( ... ) {
        CERROR << "Error occur onHttp.";
    }

        //websocketpp::http::parser::request rt = con->get_request();
        //const string& strUri = rt.get_uri();
        //const string& strMethod = rt.get_method();
        //const string& strBody = rt.get_body();
        //const string& strVersion = rt.get_version();
        //CINFO << strMethod.c_str() << ":" <<strUri.c_str() << ":" << strBody << ":" << strVersion;

        /*
        con->set_body("everything is ok now!");
        con->set_status(websocketpp::http::status_code::value(600));//websocketpp::http::status_code::ok

        websocketpp::http::parser::response rp;
        string strContent = rt.raw();
        rp.consume(strContent.c_str(), strContent.size());
        //if ( strMethod.compare("POST") == 0 )	{
            websocketpp::http::parser::request r;
            size_t nRet = r.consume(strUri.c_str(), strUri.size());
            int k = 0;
        //}
        */
}

void WebSocket::send()
{

}

void WebSocket::addHandler(std::string const& url, URLHandler const& handler)
{
    CINFO << "Add url handler :" << url;
    m_urlHandlers.insert(std::make_pair(url, handler));
}

} // namespace rpc

