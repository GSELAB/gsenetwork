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
#include <vector>
#include <string>

#include <core/Block.h>
#include <core/Account.h>
#include <core/Guards.h>
#include <database/DatabaseController.h>
#include <core/Producer.h>
#include <core/Storage.h>

using namespace database;
using namespace core;

namespace runtime {
namespace storage {
class Repository { //: public std::enable_shared_from_this<Repository> {
public:
    Repository(Block& block, DatabaseController *dbc): m_block(block), m_dbc(dbc) {}

    Repository(Block& block, std::shared_ptr<Repository> parent, DatabaseController *dbc):
        m_block(block), m_parent(parent), m_dbc(dbc) {}

    ~Repository() { if (m_parent) m_parent.reset(); }

    void setParentNULL() { m_parent.reset(); }

    Account getAccount(Address const& address);

    uint64_t getBalance(Address const& address);

    void addBalance(Address const& address, uint64_t amount);

    void subBalance(Address const& address, uint64_t amount);

    void setBalance(Address const& address, uint64_t value);

    void transfer(Address const& from, Address const& to, uint64_t value);

    void burn(Address const& target, uint64_t value);

    void bonus(Address const& target, uint64_t value);

    void put(Account const& account);

    Block& getBlock() { return m_block; }

    void put(Block const& block);

    Producer getProducer(Address const& address);

    std::map<Address, Producer> getProducerList() const;

    void put(Producer const& producer);

    void clearVote(Address const& address);

    void voteIncrease(Address const& voter, Address const& candidate, uint64_t value);

    Transaction getTransaction(TxID const& id);

    void put(Transaction& tx);

    Block getBlock(BlockID const& blockID);

    size_t codeSize(Address const& address);

    h256 codeHash(Address const& address);

    bytes code(Address const& address);

    u256 getStorageValue(Address const& address, u256 const& key);

    void setStorageValue(Address const& address, u256 const& key, u256 const& value);

    void commit();

public:
    bool isAddressExist(Address const& address);

    bool isContractAddress(Address const& address);

private:
    DatabaseController *m_dbc;
    std::shared_ptr<Repository> m_parent = nullptr;

    mutable Mutex x_mutexAccount;
    std::map<Address, Account> m_cacheAccount;

    mutable Mutex x_mutexProducer;
    std::map<Address, Producer> m_cacheProducer;

    mutable Mutex x_mutexTransaction;
    std::map<TxID, Transaction> m_cacheTransaction;

    mutable Mutex x_mutexStorage;
    std::map<Address, Storage> m_cacheStorage;

    Block m_block;
};
} // end namespace storage
} // end namespace of runtime
