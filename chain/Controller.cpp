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

#include <boost/thread.hpp>
#include <iostream>

#include <chain/Controller.h>

using namespace database;

namespace chain {

Controller::Controller()
{

}

Controller::~Controller()
{
    // delete m_net;
}

namespace {
void my_func()
{
    std::cout << "GSE TEST!" << std::endl;
}
}

void Controller::init()
{
    m_net = NetController::getInstance();
    m_dbc = new DatabaseController();
    m_dbc->init();

    boost::thread t(my_func);
    t.join();
}

static Controller *controller = nullptr;
Controller* toController()
{

    if (!controller) {
        return new Controller();
    }

    return controller;
}

void delController() {
    if (controller) {
        delete controller;
        controller = nullptr;
    }
}

}