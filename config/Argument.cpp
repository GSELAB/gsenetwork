#include <fstream>
#include <iostream>
#include <assert.h>
#include <vector>

#include <config/Argument.h>
#include <json/json.h>
#include <net/Network.h>
#include <core/Log.h>

using namespace std;
using namespace core;

namespace config {

#define DEFAULT_CONFIG_DIR "./default_config"

Argument ARGs;

void initArgument()
{
    Json::Reader reader;
    Json::Value root;
    ifstream fin(DEFAULT_CONFIG_DIR, ios::binary);
    assert(fin.is_open());
    assert(reader.parse(fin, root));

    Json::Value rpc = root["rpc"];
    Json::Value local = root["local"];
    Json::Value net = root["net"];
    Json::Value key = root["key"];
    Json::Value node = root["node"];
    Json::Value genesis = root["genesis"];

    bi::tcp::endpoint rpcEP = net::Network::resolveHost(rpc["ip_port"].asString());
    bi::tcp::endpoint localEP = net::Network::resolveHost(local["local_ip_port"].asString());
    ARGs.m_rpc = { rpcEP.address(), rpcEP.port(), rpcEP.port() };
    ARGs.m_local = { localEP.address(), localEP.port(), localEP.port() };
    ARGs.m_secret = Secret(key["secret"].asString());

    Json::Value nodeList = net["node_list"];
    for (unsigned i = 0; i < nodeList.size(); i++) {
        bi::tcp::endpoint ep = net::Network::resolveHost(nodeList[i].asString());
        Host h(ep.address(), ep.port(), ep.port());
        ARGs.m_neighbors.push_back(h);
    }

    Json::Value trustNodeList = net["trust_node_list"];
    for (unsigned i = 0; i < trustNodeList.size(); i++) {
        bi::tcp::endpoint ep = net::Network::resolveHost(trustNodeList[i].asString());
        Host h(ep.address(), ep.port(), ep.port());
        ARGs.m_trustNeighbors.push_back(h);
    }

    ARGs.m_syncFlag = node["sync"].asBool();
    chain::initGenesis(ARGs.m_genesis, genesis);

    fin.close();
}
} // namespace config