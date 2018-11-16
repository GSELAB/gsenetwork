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

#include "chain/SubChainController.h"

namespace chain {

SubChainController::SubChainController()
{}

SubChainController::SubChainController(ChainID id)
{
    m_subChainID = id;
}

SubChainController::~SubChainController()
{

}

void SubChainController::init()
{

}

void SubChainController::setSubChainID(ChainID id)
{
    m_subChainID = id;
}

uint64_t SubChainController::getSubChainID()
{
    return m_subChainID;
}

} // end of namespace chain