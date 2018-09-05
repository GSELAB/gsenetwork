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

#include <core/Controller.h>
#include <core/Types.h>
#include <core/Block.h>
#include <core/Transaction.h>
#include <storage/Repository.h>

namespace chain {

class BlockChain {
public:
    struct MemoryItem {
        uint64_t m_blockNumber;
        std::shared_ptr<Reposity> m_repository;

        MemoryItem(uint64_t number, std::shared_ptr<Reposity> repository): m_blockNumber(number), m_repository(repository) {}
    };

public:
    BlockChain(): m_controller(&controller), m_chainID(DEFAULT_GSE_NETWORK) { }

    BlockChain(Controller* controller): m_controller(controller), m_chainID(DEFAULT_GSE_NETWORK) { }

    BlockChain(Controller* controller, ChainID const& chainID): m_controller(controller), m_chainID(chainID) { }

    ~BlockChain();

    Controller* getController() const { return m_controller; }

    ChainID const& getChainID() const { return m_chainID; }

    void setChainID(ChainID const& chainID) { m_chainID = chainID; }

    bool processBlock(Block const& block);

    bool processTransaction(Block const& block, Transaction const& transaction);

    bool processTransaction(Transaction const& transaction);

    bool checkBifurcation();

private:
    Controller* m_controller;

    ChainID m_chainID;

    std::queue<MemoryItem> m_queueMemoryBlockChain;

};
} // end namespace