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

#include "bundle/AccountBundle.h"

namespace bundle {

AccountBundle::AccountBundle(char *data)
{

}

AccountBundle::AccountBundle(pchain::Account &account)
{

}

AccountBundle::AccountBundle(AccountBundle &accountBundle)
{

}

AccountBundle::~AccountBundle()
{
    // delete Account
}

uint64_t AccountBundle::getBalance()
{
    return -1;
}

void AccountBundle::setBalance(uint64_t balance)
{

}

char *AccountBundle::getData()
{
    return NULL;
}

} // end of namespace bundle