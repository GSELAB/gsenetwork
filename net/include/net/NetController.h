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

#include <queue>

#include <core/Block.h>
#include <core/Transaction.h>
#include <config/NetConfig.h>

namespace net {

class Host;

class NetController {
public:
    NetController(): m_inited(false) {}
    NetController(config::NetConfig const& netConfig);

    ~NetController();

    void init();

    void broadcast(char *msg);

    void broadcast(std::shared_ptr<core::Transaction> tMsg);

    void broadcast(std::shared_ptr<core::Block> bMsg);

    // std::queue<TransactionBundle> getTransactionCache();

    std::shared_ptr<core::Transaction> getTransactionFromCache();

    std::shared_ptr<core::Block> getBlockFromCache();


private:
    bool m_inited;

    Host* m_host = nullptr;

    std::queue<std::shared_ptr<core::Transaction>> transactionsQueue;

    std::queue<std::shared_ptr<core::Block>> blocksQueue;
};

} // end of namespace