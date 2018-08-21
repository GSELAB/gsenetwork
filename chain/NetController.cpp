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

#include "chain/NetController.h"

namespace chain {

NetController *NetController::netController = nullptr;

void NetController::broadcast(char *msg)
{

}

void NetController::broadcast(std::shared_ptr<bundle::TransactionBundle> tMsg)
{

}

void NetController::init()
{
    if (!isInit) {

        const char *const localhost = "127.0.0.1";
        NetworkConfig conf(localhost, 0, false);

        host = new Host("GSEGeneisNN", conf);
        host.start();
        host.listenPort();



        isInit = true;
    }
}

void NetController::broadcast(std::shared_ptr<bundle::BlockBundle> bMsg)
{

}

std::shared_ptr<bundle::TransactionBundle> NetController::getTransactionFromCache()
{
    std::shared_ptr<bundle::TransactionBundle> ret = nullptr;

    if (!this->transactionsQueue.empty()) {
        ret = this->transactionsQueue.front();
        this->transactionsQueue.pop();
    }

    return ret;
}

std::shared_ptr<bundle::BlockBundle> NetController::getBlockFromCache()
{
    std::shared_ptr<bundle::BlockBundle> ret = nullptr;

    if (!transactionsQueue.empty()) {
        ret = blocksQueue.front();
        blocksQueue.pop();
    }

    return ret;
}

} // end of namespace