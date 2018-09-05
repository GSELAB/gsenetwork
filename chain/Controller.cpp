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
using namespace net;

namespace chain {

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

Controller controller(DEFAULT_GSE_NETWORK);

} /* end namespace */