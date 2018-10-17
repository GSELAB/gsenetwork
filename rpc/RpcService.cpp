/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of GSENetwork.
 *
 * GSENetwork is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

#include <rpc/RpcService.h>
#include <core/Log.h>
#include <config/NetConfig.h>

namespace rpc {

RpcService::RpcService(WebSocketEventHandlerFace* face)
{
    CINFO << "RpcService::RpcService";
    m_server = new WebSocket(face, DEFAULT_LOCAL_RPC_PORT);
}

RpcService::~RpcService()
{
    CINFO << "RpcService::~RpcService";
    if (m_server) {
        m_server->shutdown();
        delete m_server;
    }
}

void RpcService::start()
{
    m_server->init();
    m_server->startService();
}

} // end of namespace rpc