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

#include "PChain.pb.h"
#include "bundle/Bundle.h"

namespace bundle {

class SubChainBundle : public Bundle {
public:
    SubChainBundle(char *data);

    SubChainBundle(pchain::SubChain &subChain);

    ~SubChainBundle();

    char *getData();

private:
    pchain::SubChain &subChain;

};

} // end of namespace bundle