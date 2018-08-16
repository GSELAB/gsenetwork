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

#include "chain/DbManager.h"

namespace chain {

DbManager::DbManager()
{

}

DbManager::~DbManager()
{

}

database::AccountStore &DbManager::getAccoutStore()
{
    return this->accoutStore;
}

database::ProducerStore &DbManager::getProducerStore()
{
    return this->producerStore;
}

database::SubChainStore &DbManager::getSubChainStore()
{
    return this->subChainStore;
}

} // end of namespace chain