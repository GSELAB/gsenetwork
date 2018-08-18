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

#include "database/All.h"

namespace chain {

class DbManager {
public:
    DbManager();

    virtual ~DbManager();

    void init();

    database::AccountStore* getAccountStore();

    database::ProducerStore* getProducerStore();

    database::SubChainStore* getSubChainStore();


private:
    database::AccountStore *accountStore;

    database::ProducerStore *producerStore;

    database::SubChainStore *subChainStore;

    // OSODatabase         m_osoDatabase;

};


} // end of namespace