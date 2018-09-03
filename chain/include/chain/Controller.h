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

#include <chain/NetController.h>
#include <database/DatabaseController.h>

namespace chain {

class Controller {
public:
    Controller();

    ~Controller();

    void init();

private:
    NetController *m_net;

    database::DatabaseController *m_dbc;
};

extern Controller* toController();
extern void delController();
}