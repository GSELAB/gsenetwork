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

#include <storage/Repository.h>
#include <core/Block.h>
#include <core/Transaction.h>

using namespace core;

namespace runtime {
namespace common {

class Runtime {
public:
    enum Type {
        PreType = 0x0,
        NormalType = 0x01,
    };

    Runtime(Transaction const& transaction, BlockPtr block, std::shared_ptr<storage::Repository> repo):
        m_block(block), m_transaction(transaction), m_repo(repo), m_type(NormalType) {}

    // @just for pre execute
    Runtime(Transaction const& transaction, std::shared_ptr<storage::Repository> repo):
        m_block(EmptyBlockPtr), m_transaction(transaction), m_repo(repo), m_type(PreType) {}

    void init();

    void excute();

    void finished();

    uint64_t getResult() const;

private:
    Type m_type;

    Transaction m_transaction;

    BlockPtr m_block;

    std::shared_ptr<storage::Repository> m_repo;

};
}
}