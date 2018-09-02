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

using namespace database;
using namespace std;
using namespace core;

DatabaseController::DatabaseController(Controller *controller):m_controller(controller)
{
    init();
}

void DatabaseController::init()
{
    m_accountStore = std::unique_ptr<Database<core::Account>>(new Database<core::Account>("./data/account"));
    m_transactionStore = std::unique_ptr<Database<core::Transaction>>(new Database<core::Transaction>("./data/transaction"));
    m_blockStore = std::unique_ptr<Database<core::Block>>(new Database<core::Block>("./data/block"));
    m_subChainStore = std::unique_ptr<Database<core::SubChain>>(new Database<core::SubChain>("./data/subchain"));
}

Account DatabaseController::getAccount(Address const& address) const
{
    return Account(m_accountStore->get(address.ref().toString()));
}

void DatabaseController::putAccount(Account const& account)
{
    m_accountStore->put(account);
}

Transaction DatabaseController::getTransaction(string const& key) const
{
    return Transaction(m_transactionStore->get(key));
}

void DatabaseController::putTransaction(Transaction const& transaction)
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

void DatabaseController::putBlock(Block const& block)
{
    m_blockStore->put(block);
}

SubChain DatabaseController::getSubChain(chain::ChainID chainID) const
{
    return SubChain();
}

void DatabaseController::putSubChain(SubChain const& subChain)
{
    m_subChainStore->put(subChain);
}