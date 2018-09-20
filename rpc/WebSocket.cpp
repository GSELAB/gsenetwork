
#include <rpc/WebSocket.h>

namespace rpc {

WebSocket::WebSocket()
{

}

WebSocket::~WebSocket()
{

}

bool WebSocket::start()
{

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

