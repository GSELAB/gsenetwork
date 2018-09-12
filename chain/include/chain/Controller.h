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
#include <crypto/Common.h>
#include <crypto/GKey.h>
#include <producer/ProducerServer.h>

namespace chain {

class Controller: public producer::ProcuderEventHandleFace {
public:
    Controller(ChainID const& chainID): m_chainID(chainID) {}

    virtual ~Controller() {}

    void init(crypto::GKey const& key);

    void exit();

    std::shared_ptr<TransactionReceipt> processTransaction(Transaction const& transaction, int64_t max_timestamp);

    void processBlock(Block const& block);

    void processTransaction(Transaction const& transaction);

    chain::ChainID getChainID() const;

    void setChainID(chain::ChainID chainID);

    // @only used by rpc module
    bool generateTransaction();

    // @only used by rpc module
    bool addTransaction(Transaction const& transaction);

    // @used by producer
    void broadcast(std::unique_ptr<Block> block);

    // @used by producer
    void processProducerEvent();

private:
    chain::ChainID m_chainID;
    crypto::GKey m_key;

    BlockChain* m_chain;
    net::NetController* m_net;
    database::DatabaseController* m_dbc;
    producer::ProducerServer* m_producerServer;


};

extern Controller controller;
} /* end namespace */