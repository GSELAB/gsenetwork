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

class BlockBundle : public Bundle<pchain::Block> {
public:
    BlockBundle() {}

    BlockBundle(char *data);

    BlockBundle(pchain::Block &block);

    BlockBundle(BlockBundle &blockBundle);

    char *getData();

    uint64_t getSize();

    pchain::Block& getInstance();

private:
    pchain::Block block;
};

} // end of namespace bundle
