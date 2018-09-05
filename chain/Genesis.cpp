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

#include <chain/Genesis.h>
#include <core/Account.h>
#include <config/Constant.h>

using namespace core;

namespace chain {

static std::vector<GenesisItem> genesisItems{
    {
        .address = Address{"0x1100110011110011001111001100111100110021"},
        .balance = 10000000000,
    },
    {
        .address = Address{"0x1100110011110011001111001100211100110011"},
        .balance = 20000000000,
    },
    {
        .address = Address{"0x1100110011110011001111201100111100110011"},
        .balance = 4000000000,
    },
    {
        .address = Address{"0x1100110011110011021111001100111100110011"},
        .balance = 50000000,
    },
};

static Genesis genesis = {
    .genesisItems = genesisItems,
};

Genesis const& getGenesis()
{
    return genesis;
}
} // end namespace