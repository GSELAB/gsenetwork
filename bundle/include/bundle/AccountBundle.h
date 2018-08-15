/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of gsenetwork.
 *
 * gsenetwork is is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

/*
 * @author guoygang <hero.gariker@gmail.com>
 * @date 2018
 */

#ifndef __BUNDLE_ACCOUNT_BUNDLE_HEADER__
#define __BUNDLE_ACCOUNT_BUNDLE_HEADER__

#include "PChain.pb.h"

#include "bundle/Bundle.h"

namespace bundle {

class AccountBundle : public Bundle {
public:
    AccountBundle(char *data);

    AccountBundle(pchain::Account &account);

    AccountBundle(AccountBundle &accountBundle);

    ~AccountBundle();

    uint64_t getBalance();

    void setBalance(uint64_t balance);

    char *getData();

private:
    pchain::Account &account;

};

} // end of namespace bundle
#endif