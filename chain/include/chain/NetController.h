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

#include <bundle/All.h>
//#include <net/Host.h>

namespace chain {

class Host;

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

    void broadcast(std::shared_ptr<bundle::TransactionBundle> tMsg);

    void broadcast(std::shared_ptr<bundle::BlockBundle> bMsg);

    // std::queue<TransactionBundle> getTransactionCache();

    std::shared_ptr<bundle::TransactionBundle> getTransactionFromCache();

    std::shared_ptr<bundle::BlockBundle> getBlockFromCache();


private:
    static NetController *netController;

    NetController() { isInit = false; }
    ~NetController() {}

    bool isInit;
    Host *host;

    std::queue<std::shared_ptr<bundle::TransactionBundle>> transactionsQueue;

    std::queue<std::shared_ptr<bundle::BlockBundle>> blocksQueue;

};

} // end of namespace