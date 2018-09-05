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

class Runtime {
public:
    Runtime(Transaction const& transaction, Block const& block);

    // @just for pre execute
    Runtime(Transaction const& transaction);

    void init();

    void excute();

    void finished();

    uint64_t getResult() const;

private:
    Transaction m_transaction;
    std::shared_ptr<storage::Repository> m_repository;

};
} // end namespace