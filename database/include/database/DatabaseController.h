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

#include <chain/Controller.h>
#include <database/Database.h>
#include <core/Account.h>
#include <core/Transaction.h>
#include <core/Block.h>
#include <core/SubChain.h>

using namespace chain;
using namespace core;
using namespace std;

namespace database {

class DatabaseController {
public:
    DatabaseController(Controller *controller);

    Account getAccount(Address const& address) const;

    void putAccount(Account const& account);

    Transaction getTransaction(string const& key) const;

    void putTransaction(Transaction const& transaction);

    Block getBlock(string const& key) const;

    Block getBlock(uint64_t blockNumber) const;

    void putBlock(Block const& block);

    SubChain getSubChain(chain::ChainID chainID) const;

    void putSubChain(SubChain const& subChain);

protected:
    void init();

private:
    Controller *m_controller;

    unique_ptr<Database<core::Account>> m_accountStore;
    unique_ptr<Database<core::Transaction>> m_transactionStore;
    unique_ptr<Database<core::Block>> m_blockStore;
    unique_ptr<Database<core::SubChain>> m_subChainStore;

};
} // namespace end