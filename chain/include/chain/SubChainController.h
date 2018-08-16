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

#include "bundle/"

namespace chain {

class SubChainController {
public:
    SubChainController();

    SubChainController(uint64_t chainId);

    ~SubChainController();

    void init();

    void setSubChainId(uint64_t id);

    uint64_t getSubChainId();


private:
    uint64_t subChainId;

    vector<int>t

};
} // end of namespace chain
