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

WebSocket::WebSocket(WebSocketEventHandlerFace* face, unsigned short listenPort, chain::ChainID chainID):
    m_face(face), m_listenPort(listenPort), m_chainID(chainID)
{
    if (m_chainID == GSE_UNKNOWN_NETWORK) {
        throw RPCException("Unknown Chain ID:" + toString(GSE_UNKNOWN_NETWORK));
    }
}

void WebSocket::registerUrlHandlers()
{
    addHandler("/get_version", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        CINFO << "get_version";
        ret = toJson("version", m_face->getVersion()).toStyledString();
        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_account", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        if (reader.parse(body, root)) {
            std::string addressString = root["address"].asString();
            Address address(addressString);
            CINFO << "Address:" << address;
            Account account = m_face->getAccount(address);
            ret = toJson(account).toStyledString();
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }

        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_producer", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        if (reader.parse(body, root)) {
            std::string addressString = root["address"].asString();
            Address address(addressString);
            CINFO << "Address:" << address;
            Producer producer = m_face->getProducer(address);
            ret = toJson(producer).toStyledString();
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }

        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_producer_list", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        Producers producerList= m_face->getCurrentProducerList();
        ret = toJson(producerList).toStyledString();
        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_active_producer_list", [this] (std::string, std::string body, URLRequestCallback urlRC) {

    });

    addHandler("/get_balance", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        if (reader.parse(body, root)) {
            std::string addressString = root["address"].asString();
            Address address(addressString);
            CINFO << "Address:" << address;
            uint64_t balance = m_face->getBalance(address);
            ret = toJson("balance", balance).toStyledString();
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }

        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_height", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        ret = toJson("height", m_height).toStyledString();
        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_solidify_height", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        ret = toJson("solidify-height", m_solidifyHeight).toStyledString();
        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_transaction", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        if (reader.parse(body, root)) {
            std::string hashString = root["txHash"].asString();
            TxID txID(hashString);
            CINFO << "Tx hash:" << txID;
            Transaction tx = m_face->getTransaction(txID);
            ret = toJson(tx).toStyledString();
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }

        urlRC(URLCode::Default, ret);
    });

    addHandler("/get_latest_transactions", [this] (std::string, std::string body, URLRequestCallback urlRC) {

    });

    addHandler("/get_block", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        if (reader.parse(body, root)) {
            uint64_t number = root["blockNumber"].asUInt64();
            CINFO << "get_block " << number;
            Block block = m_face->getBlockByNumber(number);
            ret = toJson(block).toStyledString();
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }

        urlRC(URLCode::Default, ret);
    });

    addHandler("/push_transaction", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        std::string ret;
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        if (reader.parse(body, root)) {
            Transaction tx(toTransaction(root));
            m_face->broadcast(tx);
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }
        urlRC(URLCode::Default, ret);
    });

    addHandler("/create_transaction", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        std::string ret;
        if (reader.parse(body, root)) {
            std::string senderString = root["sender"].asString();
            std::string recipientString = root["recipient"].asString();
            uint64_t value = root["value"].asUInt64();
            Transaction tx;
            tx.setChainID(m_chainID);
            tx.setType(Transaction::TransferType);
            tx.setSender(Address(senderString));
            tx.setRecipient(Address(recipientString));
            tx.setTimestamp(currentTimestamp());
            tx.setValue(value);
            ret = toJson(tx).toStyledString();
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }
        urlRC(URLCode::Default, ret);
    });

    addHandler("/create_producer", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        CINFO << "/create_producer";
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        std::string ret;
        if (reader.parse(body, root)) {
            std::string senderString = root["sender"].asString();
            Transaction tx(EmptyTransaction);
            tx.setChainID(m_chainID);
            tx.setType(Transaction::BeenProducerType);
            tx.setSender(Address(senderString));
            tx.setTimestamp(currentTimestamp());
            ret = toJson(tx).toStyledString();
        } else {
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }
        urlRC(URLCode::Default, ret);
    });

    addHandler("/vote", [this] (std::string, std::string body, URLRequestCallback urlRC) {
        Json::Reader reader(Json::Features::strictMode());
        Json::Value root;
        std::string ret;
        if(reader.parse(body, root)) {
            Address sender(root["sender"].asString());
            std::string dataString = root["data"].asString();

            Transaction tx;
            tx.setChainID(m_chainID);
            tx.setType(Transaction::VoteType);
            tx.setSender(sender);
            tx.setTimestamp(currentTimestamp());
            tx.setData(core::js_util::jsToBytes(dataString));
            ret = toJson(tx).toStyledString();
        }else{
            ret = "Parse body failed, invalid format.\n";
            CINFO << ret;
        }
        urlRC(URLCode::Default, ret);
    });
}

void WebSocket::registerObservers()
{
    m_face->registerObserver(Observer<Object*>([this] (Object* object) {
        switch (object->getObjectType()) {
            case Object::BlockHeightType: {
                BlockHeight *height = dynamic_cast<BlockHeight*>(object);
                if (height) {
                    m_height = height->getNumber();
                }
                break;
            }
            case Object::SolidifyBlockHeightType: {
                SolidifyBlockHeight *solidifyHeight = dynamic_cast<SolidifyBlockHeight*>(object);
                m_solidifyHeight = solidifyHeight->getNumber();
                break;
            }
            case Object::TransactionType: {
                break;
            }
            case Object::BlockType: {
                break;
            }
            case Object::ProducerSnapshotType: {
                break;
            }
            case Object::ActiveProducerSnapshotType: {
                break;
            }
            default: {
                break;
            }
        }
    }));
}

bool WebSocket::init()
{
    registerObservers();
    registerUrlHandlers();

    m_height = m_face->getHeight();
    m_solidifyHeight = m_face->getSolidifyHeight();

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
                m_rpcServer.run();
        }
    } catch(websocketpp::exception const& e) {
        CERROR << "RPC server run failed, " << e.what();
    }
}

bool WebSocket::shutdown()
{
    m_rpcServer.stop();
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

    } catch(Exception& e) {
        CERROR << "Error occur onHttp - " << e.what();
    } catch (websocketpp::exception& e) {
        CERROR << "Error occur onHttp - " << e.what();
    } catch (std::exception& e) {
        CERROR << "Error occur onHttp - " << e.what();
    }
}
}

