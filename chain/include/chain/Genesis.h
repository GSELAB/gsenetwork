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

#pragma

#include <core/Address.h>
#include <core/Common.h>

namespace chain {

struct GenesisItem {
    const core::Address address;
    uint64_t balance;
};

struct Genesis {
    std::vector<GenesisItem> genesisItems ;
};

extern Genesis const& getGenesis();

} // end namespace