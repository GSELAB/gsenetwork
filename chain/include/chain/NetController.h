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

//#include <bundle/All.h>
//#include <net/Host.h>

#include <core/Block.h>
#include <core/Transaction.h>

namespace chain {

class NetController {
public:
    static NetController* getInstance()
    {
        if (!netController) {
            netController = new NetController();
        }

        return netController;
    }

    void init();

    void broadcast(char *msg);

    void broadcast(std::shared_ptr<core::Transaction> tMsg);

    void broadcast(std::shared_ptr<core::Block> bMsg);

    // std::queue<TransactionBundle> getTransactionCache();

    std::shared_ptr<core::Transaction> getTransactionFromCache();

    std::shared_ptr<core::Block> getBlockFromCache();


private:
    static NetController *netController;

    NetController() { isInit = false; }
    ~NetController() {}

    bool isInit;


    std::queue<std::shared_ptr<core::Transaction>> transactionsQueue;

    std::queue<std::shared_ptr<core::Block>> blocksQueue;

};

} // end of namespace