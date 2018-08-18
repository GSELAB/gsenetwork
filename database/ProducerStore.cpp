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

#include "database/ProducerStore.h"

namespace database {

ProducerStore::ProducerStore()
{

}

ProducerStore::~ProducerStore()
{

}

bundle::ProducerBundle& ProducerStore::getProducer(char *address)
{
    return *(new bundle::ProducerBundle());
}

void ProducerStore::putProducer(bundle::ProducerBundle &producerBundle)
{

}

} // end of namespace database