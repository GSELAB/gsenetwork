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

#pragma once

#include "PChain.pb.h"
#include "bundle/Bundle.h"

namespace bundle {

class ProducerBundle : public Bundle<pchain::Producer> {
public:
    ProducerBundle();

    ProducerBundle(char *data);

    ProducerBundle(pchain::Producer &producer);

    ProducerBundle(ProducerBundle &producerBundle);

    char *getData();

    uint64_t getSize();

    pchain::Producer& getInstance();

private:
    pchain::Producer producer;
};

} // end of namespace bundle