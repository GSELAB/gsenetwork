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
#include <string>

#include <core/Block.h>
#include <core/Account.h>
#include <core/Guards.h>
#include <database/DatabaseController.h>
#include <core/Producer.h>

using namespace database;
using namespace core;

namespace runtime {
namespace storage {
class Repository { //: public std::enable_shared_from_this<Repository> {
public:
    Repository(std::shared_ptr<Block> block): m_block(block) {}

    Repository(std::shared_ptr<Block> block, std::shared_ptr<Repository> parent): m_block(block), m_parent(parent) {}

    void setParentNULL() { m_parent.reset(); }

    Account const& getAccount(Address const& address) const;

    bool transfer(Address const& from, Address const& to, uint64_t value);

    bool burn(Address const& target, uint64_t value);

    void putAccount(Account const& account);

    Block getBlock() { return *m_block; }

    Producer getProducer(Address const& address) const;

    void addProducer(Producer const& producer);

    void voteIncrease(Address const& voter, Address const& candidate, uint64_t value);

    void voteDecrease(Address const& voter, Address const& candidate, uint64_t value);

    void commit();

private:
    DatabaseController *m_db;
    std::shared_ptr<Repository> m_parent = nullptr;

    mutable Mutex x_accountCache;
    mutable std::unordered_map<Address, Account> m_accountCache;

    mutable Mutex x_producerCache;
    mutable std::unordered_map<Address, Producer> m_producerCache;

    std::shared_ptr<Block> m_block;
};
} // end namespace storage
} // end namespace of runtime

