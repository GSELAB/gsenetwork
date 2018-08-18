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

class AccountBundle : public Bundle<pchain::Account> {
public:
    AccountBundle(const std::string& data);

    AccountBundle(pchain::Account &account);

    AccountBundle(AccountBundle &accountBundle);

    ~AccountBundle();

    uint64_t getBalance();

    void setBalance(uint64_t balance);

    string getData();

    pchain::Account& getInstance();

private:
    pchain::Account account;

};

using AccountBundlePtr = std::shared_ptr<AccountBundle>;

} // end of namespace bundle