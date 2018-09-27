#include <string>
#include <rpc/WebSocket.h>
#include <core/Log.h>

using namespace std;

namespace rpc {

namespace url_handler {

}

#define CALL()

enum URLCode {
    Default = 200,
    GetVersion = 200,

};

void WebSocket::registerUrlHandlers()
{
    // get
    addHandler("/get_version", [&](std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "/get_version";
        urlRC(URLCode::Default, m_face->getVersion() + "\n");
    });
    // addHandler("getinfo", );

    addHandler("/create_transaction", [&](std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "/create_transaction";
        // TODO: Add content here
        std::string ret("undefined\n");
        {

        }

        urlRC(URLCode::Default, ret);
    });

    addHandler("/push_transaction", [&](std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "/push_transaction";
        // TODO: Add content here
        std::string ret("undefined\n");
        {

        }
        urlRC(URLCode::Default, ret);
    });
}

void on_message(RpcServer* server, ConnectHDL hdl, MessagePtr msg)
{
    server->send(hdl, msg->get_payload(), msg->get_opcode());
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
        bi::tcp::endpoint const ep = con->get_socket().lowest_layer().local_endpoint();

        Request const& req = con->get_request();
        std::string const& host = req.get_header("Host");
        if (host.empty()) {
            CERROR << "host is empty";
            con->set_status(websocketpp::http::status_code::bad_request);
                return;
        }

        con->append_header("Content-type", "application/json");
        string const body = con->get_request_body();
        string const resource = con->get_resource();

        auto handler_itr = m_urlHandlers.find(resource);
        if (handler_itr != m_urlHandlers.end()) {
            handler_itr->second(resource, body, [con](int code, std::string body) {
                con->set_body(body);
                con->set_status(websocketpp::http::status_code::value(code));
                // con->send_http_response();
            });
        } else {
            CERROR << "Not found " << resource;
            con->set_body("Not found " + resource + "\n");
            con->set_status(websocketpp::http::status_code::not_found);
        }

    } catch( ... ) {
        CERROR << "Error occur onHttp.";
    }
}

void WebSocket::send()
{

}

void WebSocket::addHandler(std::string const& url, URLHandler const& handler)
{
    // CINFO << "Add url handler :" << url;
    m_urlHandlers.insert(std::make_pair(url, handler));
}

} // namespace rpc

