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

#include <unordered_map>

#include <core/Account.h>
#include <database/DatabaseController.h>

using namespace database;
using namespace core;

namespace runtime {
namespace storage {
class Repository {
public:
    // virtual std::shared_ptr<Account> getAccount() const = 0;
    Repository();

    Account const& getAccount(Address const& address) const;

    bool transfer(Address const& from, Address const& to, uint64_t value);

    void putAccount(Account const& account);

private:
    DatabaseController *m_db;
    const Account EmptyAccount;
    mutable std::unordered_map<Address, Account> m_accountCache;
};
} // end namespace storage
} // end namespace of runtime

