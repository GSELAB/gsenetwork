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

#include <chain/BlockChain.h>
#include <runtime/Runtime.h>

using namespace core;

namespace chain {



bool BlockChain::processBlock(Block const& block)
{
    bool ret;

    /* DO CHECK BLOCK HEADER and TRANSACTIONS (PARALLEL) */
    {

    }

    for (auto const& item : block.getTransactions())
        if (!processTransaction(block, item))
            ret = false;

    return ret;
}

bool BlockChain::processTransaction(Block const& block, Transaction const& transaction)
{

    //Runtime runtime()
    return true;
}

bool BlockChain::processTransaction(Transaction const& transaction)
{
    // Runtime runtime();
    return false;
}

bool BlockChain::checkBifurcation()
{

    return true;
}
} // end namespace