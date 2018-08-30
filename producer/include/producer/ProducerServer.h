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

#include<vector>

#include <chain/DbManager.h>
#include <chain/NetController.h>
#include <producer/Producer.h>
#include <core/Account.h>

namespace producer {

/* The producer's state. */
enum ProducerState {
    Ready,
    Running,
    Suspend,
    Stop,
    Undefined
};

enum ProducerRet {
    PR_SUCCESS,
    PR_FAILED,
    PR_UNDEFINED,
};

class ProducerServer {
public:
    ProducerServer();

    ProducerServer(chain::DbManager &dbManager);

    ~ProducerServer();

    int init();

    int start();

    int suspend();

    int stop();

    void generateBlock();

private:
    std::vector<std::shared_ptr<Producer>> producers;

    chain::DbManager *dbManager;
    chain::NetController *netController;


    ProducerState state;

    std::shared_ptr<core::Account> currentAccout;

    uint64_t chainId;
};

} // end of namespace