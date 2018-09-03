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

#include <string>
#include <memory>

#include <database/Database.h>
#include <core/Account.h>
#include <core/Transaction.h>
#include <core/Block.h>
#include <core/SubChain.h>

using namespace core;
using namespace std;

namespace database {

class DatabaseController {
public:
    DatabaseController();

    Account getAccount(Address const& address) const;

    void putAccount(Account& account);

    Transaction getTransaction(string const& key) const;

    void putTransaction(Transaction& transaction);

    Block getBlock(string const& key) const;

    Block getBlock(uint64_t blockNumber) const;

    void putBlock(Block& block);

    SubChain getSubChain(chain::ChainID chainID) const;

    void putSubChain(SubChain& subChain);

    void init();

private:
//    Controller *m_controller;

    unique_ptr<Database> m_accountStore;
    unique_ptr<Database> m_transactionStore;
    unique_ptr<Database> m_blockStore;
    unique_ptr<Database> m_subChainStore;

};
} // namespace end