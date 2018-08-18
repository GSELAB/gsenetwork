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

#include "database/Database.h"
#include "bundle/ProducerBundle.h"

namespace database {

class ProducerStore : public Database {
public:
    ProducerStore();

    ~ProducerStore();

    bundle::ProducerBundle &getProducer(char* address);

    void putProducer(bundle::ProducerBundle& producerBundle);

private:

};

} // end of namespace database