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

#include <core/Transaction.h>
#include <core/Block.h>
#include <storage/Repository.h>

using namespace core;
using namespace runtime::storage;

namespace runtime {

class Runtime {
public:
    Runtime(Transaction const& transaction, Block const& block, std::shared_ptr<Repository> repository);

    void init();

    void excute();

    void finished();

    uint64_t getResult() const;

private:
    Transaction m_transaction;
    std::shared_ptr<Repository> m_repository;

};
} // end namespace