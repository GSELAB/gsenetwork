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

#include <database/DatabaseController.h>
#include <chain/Controller.h>
#include <core/Log.h>

using namespace std;
using namespace core;
using namespace chain;

namespace database {

DatabaseController::DatabaseController()
{

}

void DatabaseController::init()
{

    CINFO << __func__ << " : create database";
    m_accountStore = std::unique_ptr<Database>(new Database("./data/account"));
    m_transactionStore = std::unique_ptr<Database>(new Database("./data/transaction"));
    m_blockStore = std::unique_ptr<Database>(new Database("./data/block"));
    m_subChainStore = std::unique_ptr<Database>(new Database("./data/subchain"));
}

Account DatabaseController::getAccount(Address const& address) const
{
    return Account(m_accountStore->get(address.ref().toString()));
}

void DatabaseController::putAccount(Account& account)
{
    m_accountStore->put(account);
}

Transaction DatabaseController::getTransaction(string const& key) const
{
    return Transaction(m_transactionStore->get(key));
}

void DatabaseController::putTransaction(Transaction& transaction)
{
    m_transactionStore->put(transaction);
}

Block DatabaseController::getBlock(string const& key) const
{
    return Block(m_blockStore->get(key));
}

Block DatabaseController::getBlock(uint64_t blockNumber) const
{
    return Block();
}

void DatabaseController::putBlock(Block& block)
{
    m_blockStore->put(block);
}

SubChain DatabaseController::getSubChain(chain::ChainID chainID) const
{
    return SubChain();
}

void DatabaseController::putSubChain(SubChain& subChain)
{
    m_subChainStore->put(subChain);
}
} // end namespace