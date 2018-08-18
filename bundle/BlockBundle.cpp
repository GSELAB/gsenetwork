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


#include "bundle/BlockBundle.h"

namespace bundle {

BlockBundle::BlockBundle(char *data)
{

}

BlockBundle::BlockBundle(pchain::Block &block)
{

}

BlockBundle::BlockBundle(BlockBundle &blockBundle)
{

}

char* BlockBundle::getData()
{
    return NULL;
}

uint64_t BlockBundle::getSize()
{
    return 0;
}

pchain::Block& BlockBundle::getInstance()
{
    return this->block;
}

} // end of namespace bundle