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

using namespace core;

namespace runtime {

class Runtime {
public:
    Runtime(Transaction const& transaction, Block const& block);

    void init();

    void excute();

    void finished();

    uint64_t getResult() const;

private:
    Transaction *transaction;

};
} // end namespace