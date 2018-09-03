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

#include <chain/Types.h>
#include <net/NetController.h>
#include <database/DatabaseController.h>
#include <core/Transaction.h>
#include <core/TransactionReceipt.h>
#include <core/Block.h>

namespace chain {

class Controller {
public:
    Controller();

    ~Controller();

    void init();

    std::shared_ptr<TransactionReceipt> processTransaction(Transaction const& transaction, int64_t max_timestamp);

    void processBlock(Block const& block);

    void checkSwitch();





private:
    net::NetController *m_net;

    database::DatabaseController *m_dbc;
};

extern Controller* toController();
extern void delController();
}