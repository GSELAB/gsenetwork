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

#include <storage/Repository.h>

using namespace core;

namespace runtime {
namespace storage {

Repository::Repository()
{

}

Account const& Repository::getAccount(Address const& address) const
{
    return EmptyAccount;
}

void Repository::putAccount(Account const& account)
{
    // store the account to cache
}

bool Repository::transfer(Address const& from, Address const& to, uint64_t value)
{
    Account const& _from = getAccount(from);
    Account const& _to = getAccount(to);

    if (_from == EmptyAccount) {
        //std::count << "from account is null";
        return false;
    }

    if (_from.getBalance() < value || _to.getBalance() + value < value)
        return false;

    (*((Account*)&_from)).setBalance(_from.getBalance() - value);
    (*((Account*)&_to)).setBalance(_to.getBalance() + value);
    putAccount(_from);
    putAccount(_to);
    return true;
}
} // end namespace storage
} // end namespace runtime