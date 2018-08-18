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

#include "chain/DbManager.h"

namespace chain {

DbManager::DbManager()
{

}

DbManager::~DbManager()
{

}

void DbManager::init()
{

}

database::AccountStore* DbManager::getAccountStore()
{
    return this->accountStore;
}

database::ProducerStore* DbManager::getProducerStore()
{
    return this->producerStore;
}

database::SubChainStore* DbManager::getSubChainStore()
{
    return this->subChainStore;
}

} // end of namespace chain