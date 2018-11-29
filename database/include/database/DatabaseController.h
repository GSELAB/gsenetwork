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
#include <config/Constant.h>
#include <core/Producer.h>
#include <core/AttributeState.h>
#include <chain/BlockState.h>

using namespace core;

namespace database {

class DatabaseController {
public:
    DatabaseController();

    ~DatabaseController();

    void init();

    void incrementBlockHeight(uint64_t incr = 1);

    void decrementBlockHeight(uint64_t decr = -1);

    Account getAccount(Address const& address) const;

    void put(Account& account);

    Producer getProducer(Address const& address) const;

    std::vector<Producer> getProducerList() const;

    void put(Producer& producer);

    Transaction getTransaction(bytes const& key) const;

    Transaction getTransaction(TxID const& key) const;

    void put(Transaction& transaction);

    Block getBlock(BlockID const& key) const;

    Block getBlock(bytes const& key) const;

    Block getBlock(uint64_t blockNumber) const;

    void put(Block& block);

    chain::BlockState getBlockState(BlockID const& key) const;

    chain::BlockState getBlockState(bytes const& key) const;

    chain::BlockState getBlockState(uint64_t key) const;

    void put(chain::BlockState& bs);

    SubChain getSubChain(chain::ChainID chainID) const;

    void put(SubChain& subChain);

    AttributeState getAttribute(bytes const& key) const;

    void putAttribute(AttributeState& t);

protected:
    bool checkGenesisExisted();

    bool initGenesis();

private:
    unique_ptr<Database> m_attributesStore;
    unique_ptr<Database> m_accountStore;
    unique_ptr<Database> m_transactionStore;
    unique_ptr<Database> m_blockStore;
    unique_ptr<Database> m_blockIndexStore;
    unique_ptr<Database> m_blockStateStore;
    unique_ptr<Database> m_blockStateIndexStore;
    unique_ptr<Database> m_subChainStore;
    unique_ptr<Database> m_producerStore;
    unique_ptr<Database> m_storageStore;

};
} // namespace end
