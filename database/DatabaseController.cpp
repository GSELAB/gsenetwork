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
#include <config/Constant.h>

using namespace std;
using namespace core;
using namespace chain;
using namespace config;

namespace database {

std::string ATTRIBUTE_DIR_FILE("./data/attributes");
std::string ACCOUNT_DIR_FILE("./data/account");
std::string TRANSACTION_DIR_FILE("./data/transaction");
std::string BLOCK_DIR_FILE("./data/block");
std::string SUBCHAIN_DIR_FILE("./data/subchain");

DatabaseController::DatabaseController()
{

}

void DatabaseController::init()
{
    m_attributesStore = std::unique_ptr<Database>(new Database(ATTRIBUTE_DIR_FILE));
    m_accountStore = std::unique_ptr<Database>(new Database(ACCOUNT_DIR_FILE));
    m_transactionStore = std::unique_ptr<Database>(new Database(TRANSACTION_DIR_FILE));
    m_blockStore = std::unique_ptr<Database>(new Database(BLOCK_DIR_FILE));
    m_subChainStore = std::unique_ptr<Database>(new Database(SUBCHAIN_DIR_FILE));
    CINFO << "Open database  success.";
    if (checkGenesisExisted()) {
        // TODO:
    } else {
        // TODO: use genesis information
        {

        }

        m_attributesStore->put(ATTRIBUTE_GENESIS_INITED);
    }


}

bool DatabaseController::checkGenesisExisted()
{
    std::string value = m_attributesStore->get(ATTRIBUTE_GENESIS_INITED.getKey());
    if (value.empty()) {
        CINFO << "The datebase not inited, try to init the genesis." << value;
        return false;
    } else {
        ConstantState<bool> state(ATTRIBUTE_GENESIS_INITED.getKey(), bytesConstRef(&value));
        if (state.getValue() == true) {
            CINFO << "The datebase has genesis init.";
        }

        return true;
    }
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