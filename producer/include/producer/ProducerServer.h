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

#include "bundle/All.h"
#include "chain/DbManager.h"

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

    ProducerServer(DbManager &dbManager);

    ~ProducerServer();

    int init();

    int start();

    int suspend();

    int stop();

    void generateBlock();

private:
    vector<bundle::Producer> producers;

    DbManager &dbManager;

    ProducerState state;

    AccountBundlePtr currentAccout;
};

} // end of namespace