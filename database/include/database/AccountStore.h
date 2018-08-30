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
//#include "bundle/AccountBundle.h"

namespace database {

class AccountStore : public Database {
public:
    AccountStore();

    ~AccountStore();

//    bundle::AccountBundle &getAccount(char *address);

//    void putAccount(bundle::AccountBundle &accountBundle);

private:
    uint8_t isOpened;

};

} // end of namespace database