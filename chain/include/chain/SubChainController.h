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

//#include "bundle/All.h"
#include <chain/Types.h>

namespace chain {

class SubChainController {
public:
    SubChainController();

    SubChainController(ChainID id);

    ~SubChainController();

    void init();

    void setSubChainID(ChainID id);

    uint64_t getSubChainID();


private:
    ChainID m_subChainID;

    //vector<int>t

};
} // end of namespace chain
