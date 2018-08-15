/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of gsenetwork.
 *
 * gsenetwork is is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

/*
 * @author guoygang <hero.gariker@gmail.com>
 * @date 2018
 */

#ifndef __BUNDLE_BLOCK_BUNDLE_HEADER__
#define __BUNDLE_BLOCK_BUNDLE_HEADER__

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

#endif