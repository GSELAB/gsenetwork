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

#include "chain/SubChainController.h"

namespace chain {

SubChainController::SubChainController()
{}

SubChainController::SubChainController(uint64_t chainId)
{
    this->subChainId = chainId;
}

SubChainController::~SubChainController()
{

}

void SubChainController::setSubChainId(uint64_t id)
{
    this->subChainId = id;
}

uint64_t SubChainController::getSubChainId()
{
    return this->subChainId;
}

} // end of namespace chain