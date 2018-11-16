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

#include <string>
#include <json/json.h>

#include <core/Address.h>
#include <core/Common.h>
#include <core/Account.h>
#include <core/Producer.h>

using namespace core;

namespace chain {

struct Genesis {
    std::map<Address, Account> m_initAccounts;
    std::map<Address, Producer> m_initProducers;
    ProducerSnapshot m_producerSnapshot;
    Block m_genesisBlock;
    bool m_inited = false;
};

extern void initGenesis(Genesis& genesis, Json::Value const& root);

} // end namespace