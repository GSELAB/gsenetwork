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

#include "bundle/ProducerBundle.h"

namespace bundle {

ProducerBundle::ProducerBundle()
{

}

ProducerBundle::ProducerBundle(char *data)
{

}

ProducerBundle::ProducerBundle(pchain::Producer &producer)
{

}

ProducerBundle::ProducerBundle(ProducerBundle &producerBundle)
{

}

char *ProducerBundle::getData()
{
    return NULL;
}

uint64_t ProducerBundle::getSize()
{
    return 0;
}

pchain::Producer& ProducerBundle::getInstance()
{
    return this->producer;
}

} // end of namespace bundle