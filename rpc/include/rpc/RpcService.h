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

#pragma once

#include <rpc/WebSocket.h>

namespace rpc {

class RpcService {
public:
    RpcService(WebSocketEventHandlerFace* face, chain::ChainID chainID = chain::GSE_UNKNOWN_NETWORK);

    ~RpcService();

    void start();

private:
    WebSocket *m_server;
};

} // end of namespace rpc