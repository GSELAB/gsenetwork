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

namespace producer {

ProducerServer::ProducerServer()
{
    this->state = Undefined;
}

ProducerServer::ProducerServer(DbManager &dbManager)
{
    this->dbManager = dbManager;

    this->state = Undefined;
}

ProducerServer::~ProducerServer()
{

}

int ProducerServer::init()
{
    if (this->state != Undefined)
        return;

    // init the procuders
    this->state = Ready;
}

int ProducerServer::start()
{
    if (this->state != Ready && this->state != Suspend)
        return -1;

    this->state = Running;
    while (this->state == Running) {
        generateBlock();
    }
}

int ProducerServer::suspend()
{
    if (this->state != Running)
        return -1;

    this->state = Suspend;
}

int ProducerServer::stop()
{
    this->state = Stop;
}

void ProducerServer::generateBlock()
{

}

} // end of namespace