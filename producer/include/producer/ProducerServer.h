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

#include <chain/Controller.h>
#include <core/Producer.h>
#include <core/Account.h>

using namespace core;

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

    ProducerServer(chain::Controller *controller);

    ~ProducerServer();

    int init();

    int start();

    int suspend();

    int stop();

    void generateBlock();

private:
    std::vector<std::shared_ptr<Producer>> m_producers;

    chain::Controller *m_controller;


    ProducerState m_state;

    std::shared_ptr<core::Account> m_currentAccout;

    chain::ChainID m_chainID;
};

} // end of namespace