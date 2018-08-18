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

#include "producer/ProducerServer.h"
#include "config/Constant.h"

namespace producer {

ProducerServer::ProducerServer()
{
    this->state = Undefined;
}

ProducerServer::ProducerServer(chain::DbManager &dbManager)
{
    this->dbManager = &dbManager;

    this->state = Undefined;
}

ProducerServer::~ProducerServer()
{

}

int ProducerServer::init()
{
    if (this->state != Undefined)
        return -1;

    // init the procuders
    this->state = Ready;
    return 0;
}

int ProducerServer::start()
{
    if (this->state != Ready && this->state != Suspend)
        return -1;

    this->state = Running;
    while (this->state == Running) {
        generateBlock();
    }

    return 0;
}

int ProducerServer::suspend()
{
    if (this->state != Running)
        return -1;

    this->state = Suspend;
    return 0;
}

int ProducerServer::stop()
{
    this->state = Stop;
    return 0;
}

void ProducerServer::generateBlock()
{
    /*
    std::shared_ptr<bundle::BlockBundle> block = std::make_shared<bundle::BlockBundle>(new bundle::BlockBundle());

    while (block->getSize() < MAX_BLOCK_SIZE) {
        //std::shared_ptr<bundle::TransactionBundle> transaction = netController->getTransactionFromCache();

    }
    */
}

} // end of namespace