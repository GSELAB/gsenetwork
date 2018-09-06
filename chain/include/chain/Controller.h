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

#include <chain/Types.h>
#include <net/NetController.h>
#include <database/DatabaseController.h>
#include <core/Transaction.h>
#include <core/TransactionReceipt.h>
#include <core/Block.h>
#include <chain/BlockChain.h>

namespace chain {

class Controller {
public:
    Controller(ChainID const& chainID): m_chainID(chainID) {}

    ~Controller() {}

    void init();

    std::shared_ptr<TransactionReceipt> processTransaction(Transaction const& transaction, int64_t max_timestamp);

    void processBlock(Block const& block);

    void processTransaction(Transaction const& transaction);

    chain::ChainID getChainID() const;

    void setChainID(chain::ChainID chainID);

    // @only used by rpc module
    bool generateTransaction();

    // @only used by rpc module
    bool addTransaction(Transaction const& transaction);

    

private:
    chain::BlockChain *m_chain;

    net::NetController *m_net;

    database::DatabaseController *m_dbc;

    chain::ChainID m_chainID;
};

extern Controller controller;
} /* end namespace */