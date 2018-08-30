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

#include <chain/Controller.h>

#include <boost/thread.hpp>
#include <iostream>

namespace chain {

Controller::Controller() {
    m_net = NetController::getInstance();
}

Controller::~Controller() {
    // delete m_net;
}

namespace {
void my_func()
{
    std::cout << "ok" << std::endl;
}
}

void Controller::init()
{
    // m_net->init();

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