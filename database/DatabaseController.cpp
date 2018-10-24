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
std::string PRODUCER_DIR_FILE("./data/producer");

DatabaseController::DatabaseController():
    m_attributesStore(std::unique_ptr<Database>(new Database(ATTRIBUTE_DIR_FILE))),
    m_accountStore(std::unique_ptr<Database>(new Database(ACCOUNT_DIR_FILE))),
    m_transactionStore(std::unique_ptr<Database>(new Database(TRANSACTION_DIR_FILE))),
    m_blockStore(std::unique_ptr<Database>(new Database(BLOCK_DIR_FILE))),
    m_subChainStore(std::unique_ptr<Database>(new Database(SUBCHAIN_DIR_FILE))),
    m_producerStore(std::unique_ptr<Database>(new Database(PRODUCER_DIR_FILE)))
{
    CINFO << "Database environment initial success";
}

DatabaseController::~DatabaseController()
{
    CINFO << "DatabaseController::~DatabaseController";
}

void DatabaseController::init()
{
    if (checkGenesisExisted()) {
        bytes key = ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getKey();
        AttributeState<uint64_t> height = getAttribute<uint64_t>(key);
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
    Genesis const& genesis = getGenesis();
    for (auto const& item : genesis.genesisItems) {
        Account account(item.address, item.balance);
        put(account);
    }

    put(ZeroBlock);
    ATTRIBUTE_CURRENT_BLOCK_HEIGHT.setValue(ZERO_BLOCK_HEIGHT);
    putAttribute<uint64_t>(ATTRIBUTE_CURRENT_BLOCK_HEIGHT);

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
    bytes value = m_accountStore->get(address.asBytes());
    return Account(bytesConstRef(&value));
}

void DatabaseController::put(Account& account)
{
    m_accountStore->put(account);
}

Producer DatabaseController::getProducer(Address const& address) const
{
    bytes value = m_producerStore->get(address.asBytes());
    return Producer(bytesConstRef(&value));
}

void DatabaseController::put(Producer& producer)
{
    m_producerStore->put(producer);
}

Transaction DatabaseController::getTransaction(bytes const& key) const
{
    return Transaction(m_transactionStore->get(key));
}

Transaction DatabaseController::getTransaction(TxID const& key) const
{
    return getTransaction(key.asBytes());
}

void DatabaseController::put(Transaction& transaction)
{
    m_transactionStore->put(transaction);
}

Block DatabaseController::getBlock(bytes const& key) const
{
    return Block(m_blockStore->get(key));
}

Block DatabaseController::getBlock(uint64_t blockNumber) const
{
    return Block();
}

void DatabaseController::put(Block& block)
{
    m_blockStore->put(block);
}

SubChain DatabaseController::getSubChain(chain::ChainID chainID) const
{
    return SubChain();
}

void DatabaseController::put(SubChain& subChain)
{
    m_subChainStore->put(subChain);
}

template<class T>
AttributeState<T> DatabaseController::getAttribute(bytes const& key) const
{
    bytes value = m_attributesStore->get(key);
    return AttributeState<T>(key, bytesConstRef(&value));
}

template<class T>
void DatabaseController::putAttribute(AttributeState<T>& t)
{
    m_attributesStore->put(t);
}
} // end namespace