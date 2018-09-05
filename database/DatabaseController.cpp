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
#include <chain/Genesis.h>
#include <core/Exceptions.h>

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
    CINFO << "Database environment initial success";
    if (checkGenesisExisted()) {
        AttributeState<uint64_t> height = getAttribute<uint64_t>(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getKey());
        CINFO << "Current block height : " << height.getValue();
    } else {
        initGenesis();
    }
}

bool DatabaseController::checkGenesisExisted()
{
    try {
        AttributeState<bool> genesisState = getAttribute<bool>(ATTRIBUTE_GENESIS_INITED.getKey());
        if (genesisState.getValue())
            CINFO << "Genesis existed";
        return genesisState.getValue();
    } catch (GSException &e) {
        CINFO << "Genesis not existed:" << e.what();
        return false;
    }
}

bool DatabaseController::initGenesis()
{
    // init account list
    Genesis const& genesis = getGenesis();
    size_t itemsCount = genesis.genesisItems.size();
    for (unsigned i; i < itemsCount; i++) {
        auto const& item = genesis.genesisItems[i];
        Account account(item.address, item.balance);
        putAccount(account);
    }

    // init attribute of block chain height
    putBlock(ZeroBlock);
    putAttribute<uint64_t>(ATTRIBUTE_CURRENT_BLOCK_HEIGHT);

    // set genesis inited flag to be true
    putAttribute<bool>(ATTRIBUTE_GENESIS_INITED);
    return true;
}

void DatabaseController::incrementBlockHeight(uint64_t incr)
{
    if (incr != 1)
        CWARN << "Increment block height:" << incr;

    if (ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue() + incr < incr)
        THROW_GSEXCEPTION("Invalid incr to increment block height");

    ATTRIBUTE_CURRENT_BLOCK_HEIGHT.setValue(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue() + incr);
    putAttribute<uint64_t>(ATTRIBUTE_CURRENT_BLOCK_HEIGHT);
}

void DatabaseController::decrementBlockHeight(uint64_t decr)
{
    if (decr != 1)
        CWARN << "Decrement block height:" << decr;

    if (ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue() < decr)
        THROW_GSEXCEPTION("Invalid decr to decrement block height");

    ATTRIBUTE_CURRENT_BLOCK_HEIGHT.setValue(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue() - decr);
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

template<class T>
AttributeState<T> DatabaseController::getAttribute(string const& key) const
{
    return AttributeState<T>(key, m_attributesStore->get(key));
}

template<class T>
void DatabaseController::putAttribute(AttributeState<T>& t)
{
    m_attributesStore->put(t);
}
} // end namespace