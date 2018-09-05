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

#include <config/Constant.h>

using namespace core;

namespace config {

AttributeState<bool> ATTRIBUTE_GENESIS_INITED(ATTRIBUTE_GENESIS_INITED_KEY, true);

AttributeState<uint64_t> ATTRIBUTE_CURRENT_BLOCK_HEIGHT(ATTRIBUTE_CURRENT_BLOCK_HEIGHT_KEY, 0);

BlockHeader ZeroBlockHeader(0);
Block ZeroBlock(ZeroBlockHeader);

} // end namespace