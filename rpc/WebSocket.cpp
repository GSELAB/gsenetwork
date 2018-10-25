#include <string>
#include <rpc/WebSocket.h>
#include <core/Log.h>
#include <core/Transaction.h>
#include <chain/Types.h>
#include <utils/Utils.h>

using namespace std;
using namespace core;
using namespace chain;
using namespace utils;

namespace rpc {

namespace url_handler {

}

#define CALL()

enum URLCode {
    Default = 200,
};

void WebSocket::registerUrlHandlers()
{
    addHandler("/get_version", [&](std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        CINFO << "get_version";
        ret = toJson("version", m_face->getVersion()).toStyledString();
        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_block", [&](std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        if (reader.parse(body, root)) {
            uint64_t number = root["blockNumber"].asInt();
            CINFO << "get_block " << number;
            Block block = m_face->getBlockByNumber(number);
            ret = toJson(block).toStyledString();
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }

        urlRC(URLCode::Default, ret);
    });

    addHandler("/push_transaction", [&](std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "/push_transaction";
        // TODO: Add content here
        std::string ret;
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        if (reader.parse(body, root))
        {
            // body to rlp transaction
            std::string str_transaction = root["transaction"].asString();
            bytes data;
            data.insert(data.begin(), str_transaction.begin(), str_transaction.end());

            Transaction transaction(data);
            m_face->broadcast(transaction);
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }
        urlRC(URLCode::Default, ret);
    });

    addHandler("/create_transaction", [&](std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "/create_transaction";
        // TODO: Add content here
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        std::string ret;
        if(reader.parse(body, root))
        {
            // body to json
            std::string str_sender = root["sender"].asString();
            std::string str_recipient = root["recipient"].asString();
            uint64_t value = root["value"].asInt();

            Transaction transaction;
            transaction.setChainID(DEFAULT_GSE_NETWORK);
            transaction.setType(Transaction::TransferType);
            transaction.setSender(Address(str_sender));
            transaction.setRecipient(Address(str_recipient));
            transaction.setValue(value);

            bytes tem = transaction.getRLPData();
            ret.insert(ret.begin(), tem.begin(),tem.end());

            ret = toJson("transfer", ret).toStyledString();
        }else{
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }
        urlRC(URLCode::Default, ret);
    });


    addHandler("/create_producer", [&](std::string, std::string body, URLRequestCallback urlRC){
        CINFO << "/create_producer";
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        std::string ret;
        if(reader.parse(body, root))
        {
            // to do
            std::string str_sender = root["sender"].asString();

            Transaction transaction;
            transaction.setChainID(DEFAULT_GSE_NETWORK);
            transaction.setType(Transaction::BeenProducerType);
            transaction.setSender(Address(str_sender));

            bytes tem = transaction.getRLPData();
            ret.insert(ret.begin(), tem.begin(), tem.end());

            ret = toJson("producer", ret).toStyledString();
        }else{
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }
        urlRC(URLCode::Default, ret);
    });
    addHandler("/create_vote", [&](std::string, std::string body, URLRequestCallback urlRC){
    CINFO << "/create_vote";

    Json::Reader reader(Json::Features::strictMode());
    Json::Value root;
    std::string ret;
    if(reader.parse(body, root))
    {
        // to do
        std::string str_sender = root["sender"].asString();
        std::string str_data = root["data"].asString();
        bytes data;
        data.insert(data.begin(), str_data.begin(), str_data.end());

        Transaction transaction;
        transaction.setChainID(DEFAULT_GSE_NETWORK);
        transaction.setType(Transaction::VoteType);
        transaction.setSender(Address(str_sender));
        transaction.setData(data);

        bytes tem = transaction.getRLPData();
        ret.insert(ret.begin(), tem.begin(), tem.end());
        ret = toJson("producer", ret).toStyledString();
    }else{
        ret = "Parse body failed, invalid format.\n";
        CINFO << ret;
    }
    urlRC(URLCode::Default, ret);
    });
}

bool WebSocket::init()
{
    registerUrlHandlers();

    try {
        m_rpcServer.clear_access_channels(websocketpp::log::alevel::all);
        m_rpcServer.clear_error_channels(websocketpp::log::alevel::all);
        m_rpcServer.init_asio();
        m_rpcServer.set_reuse_addr(true);
        m_rpcServer.set_message_handler([&](ConnectHDL hdl, MessagePtr msg) {
            onMessage(&m_rpcServer, hdl, msg);
        });

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
    CINFO << "WebSocket::onMessage";
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
            CERROR << "Host is empty";
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
            con->set_body("Undefined " + resource + "\n");
            con->set_status(websocketpp::http::status_code::not_found);
        }

    } catch( ... ) {
        CERROR << "Error occur onHttp.";
    }
}

void WebSocket::send()
{

}
} // namespace rpc

