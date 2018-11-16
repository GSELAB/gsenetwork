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
#include <chain/Genesis.h>
#include <core/Exceptions.h>
#include <config/Argument.h>
#include <core/JsonHelper.h>

using namespace std;
using namespace core;
using namespace chain;
using namespace config;

namespace database {

std::string ATTRIBUTE_DIR_FILE("./data/attributes");
std::string ACCOUNT_DIR_FILE("./data/account");
std::string TRANSACTION_DIR_FILE("./data/transaction");
std::string BLOCK_DIR_FILE("./data/block");
std::string BLOCK_INDEX_DIR_FILE("./data/block-index");
std::string BLOCK_STATE_DIR_FILE("./data/block-state");
std::string BLOCK_STATE_INDEX_DIR_FILE("./data/block-state-index");
std::string SUBCHAIN_DIR_FILE("./data/subchain");
std::string PRODUCER_DIR_FILE("./data/producer");

DatabaseController::DatabaseController():
    m_attributesStore(std::unique_ptr<Database>(new Database(ATTRIBUTE_DIR_FILE))),
    m_accountStore(std::unique_ptr<Database>(new Database(ACCOUNT_DIR_FILE))),
    m_transactionStore(std::unique_ptr<Database>(new Database(TRANSACTION_DIR_FILE))),
    m_blockStore(std::unique_ptr<Database>(new Database(BLOCK_DIR_FILE))),
    m_blockIndexStore(std::unique_ptr<Database>(new Database(BLOCK_INDEX_DIR_FILE))),
    m_blockStateStore(std::unique_ptr<Database>(new Database(BLOCK_STATE_DIR_FILE))),
    m_blockStateIndexStore(std::unique_ptr<Database>(new Database(BLOCK_STATE_INDEX_DIR_FILE))),
    m_subChainStore(std::unique_ptr<Database>(new Database(SUBCHAIN_DIR_FILE))),
    m_producerStore(std::unique_ptr<Database>(new Database(PRODUCER_DIR_FILE))) {}

DatabaseController::~DatabaseController()
{
    CWARN << "DatabaseController::~DatabaseController";
}

void DatabaseController::init()
{
    if (checkGenesisExisted()) {
        try {
            ATTRIBUTE_CURRENT_BLOCK_HEIGHT = getAttribute(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getKey());
            ATTRIBUTE_PREV_PRODUCER_LIST = getAttribute(ATTRIBUTE_PREV_PRODUCER_LIST.getKey());
            ATTRIBUTE_CURRENT_PRODUCER_LIST = getAttribute(ATTRIBUTE_CURRENT_PRODUCER_LIST.getKey());
            ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST = getAttribute(ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST.getKey());
            CWARN << "Current block height : " << ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue();
        } catch (AttributeStateException& e) {
            CERROR << e.what();
        } catch (Exception& e) {
            CERROR << e.what();
        }
    } else {
        initGenesis();
    }
}

bool DatabaseController::checkGenesisExisted()
{
    try {
        AttributeState genesisState = getAttribute(ATTRIBUTE_GENESIS_INITED.getKey());
        if (genesisState.getValue() != 0) {
            CINFO << "Genesis status - " << (bool)genesisState.getValue();
        }
        return (bool)genesisState.getValue();
    } catch (AttributeStateException& e) {
        CWARN << "Genesis not existed - " << e.what();
        return false;
    } catch (Exception& e) {
        CINFO << "Genesis check - " << e.what();
        return false;
    }
}

bool DatabaseController::initGenesis()
{
    Genesis& genesis = ARGs.m_genesis;
    for (auto& itr : genesis.m_initAccounts) {
        put(itr.second);
    }

    for (auto& itr : genesis.m_initProducers) {
        put(itr.second);
    }

    put(genesis.m_genesisBlock);
    ATTRIBUTE_CURRENT_BLOCK_HEIGHT.setValue(0);
    putAttribute(ATTRIBUTE_CURRENT_BLOCK_HEIGHT);
    putAttribute(ATTRIBUTE_GENESIS_INITED);
    ATTRIBUTE_PREV_PRODUCER_LIST.setData(genesis.m_producerSnapshot.getRLPData());
    ATTRIBUTE_CURRENT_PRODUCER_LIST.setData(genesis.m_producerSnapshot.getRLPData());
    putAttribute(ATTRIBUTE_PREV_PRODUCER_LIST);
    putAttribute(ATTRIBUTE_CURRENT_PRODUCER_LIST);
    return true;
}

void DatabaseController::incrementBlockHeight(uint64_t incr)
{
    if (incr != 1)
        CWARN << "Increment block height:" << incr;

    if (ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue() + incr < incr)
        THROW_GSEXCEPTION("Invalid incr to increment block height");

    ATTRIBUTE_CURRENT_BLOCK_HEIGHT.setValue(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue() + incr);
    putAttribute(ATTRIBUTE_CURRENT_BLOCK_HEIGHT);
}

void DatabaseController::decrementBlockHeight(uint64_t decr)
{
    if (decr != 1)
        CWARN << "Decrement block height:" << decr;

    if (ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue() < decr)
        THROW_GSEXCEPTION("Invalid decr to decrement block height");

    ATTRIBUTE_CURRENT_BLOCK_HEIGHT.setValue(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue() - decr);
    putAttribute(ATTRIBUTE_CURRENT_BLOCK_HEIGHT);
}

Account DatabaseController::getAccount(Address const& address) const
{
    bytes data = m_accountStore->get(address.asBytes());
    if (data == EmptyBytes) return EmptyAccount;
    return Account(bytesConstRef(&data));
}

void DatabaseController::put(Account& account)
{
    m_accountStore->put(account);
}

Producer DatabaseController::getProducer(Address const& address) const
{
    bytes data = m_producerStore->get(address.asBytes());
    if (data == EmptyBytes) return EmptyProducer;
    return Producer(bytesConstRef(&data));
}

std::vector<Producer> DatabaseController::getProducerList() const
{
    std::vector<bytes> bytesProducerList = m_producerStore->getAll();
    std::vector<Producer> producerList;
    for (auto const& i : bytesProducerList) {
        producerList.push_back(Producer(bytesConstRef(&i)));
    }
    return producerList;
}

void DatabaseController::put(Producer& producer)
{
    m_producerStore->put(producer);
}

Transaction DatabaseController::getTransaction(bytes const& key) const
{
    bytes data = m_transactionStore->get(key);
    if (data == EmptyBytes) return EmptyTransaction;
    return Transaction(data);
}

Transaction DatabaseController::getTransaction(TxID const& key) const
{
    return getTransaction(key.asBytes());
}

void DatabaseController::put(Transaction& transaction)
{
    m_transactionStore->put(transaction);
}

Block DatabaseController::getBlock(BlockID const& key) const
{
    return getBlock(key.asBytes());
}

Block DatabaseController::getBlock(bytes const& key) const
{
    bytes data = m_blockStore->get(key);
    if (data == EmptyBytes) return EmptyBlock;
    return Block(data);
}

Block DatabaseController::getBlock(uint64_t blockNumber) const
{
    bytes data = m_blockIndexStore->get(blockNumber);
    if (data == EmptyBytes) return EmptyBlock;
    return Block(data);
}

void DatabaseController::put(Block& block)
{
    m_blockStore->put(block);
    m_blockIndexStore->put(block.getNumber(), block.getRLPData());
}

chain::BlockState DatabaseController::getBlockState(BlockID const& key) const
{
    return getBlockState(key.asBytes());
}

chain::BlockState DatabaseController::getBlockState(bytes const& key) const
{
    bytes data = m_blockStateStore->get(key);
    if (data == EmptyBytes) return chain::EmptyBlockState;
    return chain::BlockState(data);
}

chain::BlockState DatabaseController::getBlockState(uint64_t key) const
{
    bytes data = m_blockStateStore->get(key);
    if (data == EmptyBytes) return chain::EmptyBlockState;
    return chain::BlockState(data);
}

void DatabaseController::put(chain::BlockState& bs)
{
    m_blockStateStore->put(bs);
    m_blockStateIndexStore->put(bs.m_blockNumber, bs.getRLPData());
}

SubChain DatabaseController::getSubChain(chain::ChainID chainID) const
{
    return SubChain();
}

void DatabaseController::put(SubChain& subChain)
{
    m_subChainStore->put(subChain);
}

AttributeState DatabaseController::getAttribute(bytes const& key) const
{
    bytes value = m_attributesStore->get(key);
    return AttributeState(key, bytesConstRef(&value));
}

void DatabaseController::putAttribute(AttributeState& t)
{
    m_attributesStore->put(t.getKey(), t.getRLPData());
}
} // end namespace
