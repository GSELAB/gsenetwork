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

/*
 * @author guoygang <hero.gariker@gmail.com>
 * @date 2018
 */

#pragma once
#include "PChain.pb.h"

#include "bundle/Bundle.h"

namespace bundle {

class BlockBundle : public Bundle {
public:
    BlockBundle(char *data);

    BlockBundle(pchain::Block &block);

    BlockBundle(BlockBundle &blockBundle);

    char *getData();

private:
    pchain::Block &block;
};

} // end of namespace bundle
