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

namespace chain {

Controller::Controller() {
    net = NetController::getInstance();
}

Controller::~Controller() {
    delete net;
}

void Controller::init()
{
    net->init();
}

} // end of namespace