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
#include <core/Log.h>

using namespace core;
using namespace runtime::storage;

namespace chain {

BlockChain::BlockChain(crypto::GKey const& key): m_key(key)
{
    if (m_controller == nullptr) m_controller = &controller;
    if (m_chainID == GSE_UNKNOWN_NETWORK) {
        // THROW_GSEXCEPTION("GSE_UNKNOWN_NETWORK");
        CWARN << "GSE_UNKNOWN_NETWORK, SET DEFAULT_GSE_NETWORK";
        m_chainID = DEFAULT_GSE_NETWORK;
    }

    m_dispatcher = new Dispatch(this);
}

BlockChain::BlockChain(crypto::GKey const& key, Controller* c, ChainID const& chainID): m_key(key), m_controller(c), m_chainID(chainID)
{
    m_dispatcher = new Dispatch(this);
}

BlockChain::~BlockChain()
{
    CINFO << "BlockChain::~BlockChain";
    if (m_dispatcher) delete m_dispatcher;
}

void BlockChain::init()
{
    CINFO << "Block chain init";
}

bool BlockChain::processBlock(Block const& block)
{
    /* DO CHECK BLOCK HEADER and TRANSACTIONS (PARALLEL) */
    {

    }

    MemoryItem item;
    {
        Guard g(x_memoryQueue);
        std::shared_ptr<runtime::storage::Repository> repository =
            std::make_shared<runtime::storage::Repository>(m_memoryQueue.back().getRepository());
        item.setBlockNumber(block.getNumber());
        item.setRepository(repository);
    }

    try {

        for (auto const& item : block.getTransactions())
            if (!processTransaction(block, item)) {
                // Record the failed
            }
    } catch (std::exception const& e) {
        CWARN << "Error occur process the block " << block.getNumber();
        return false;
    }

    item.setDone();
    {
        Guard g(x_memoryQueue);
        m_memoryQueue.push(item);
    }

    return true;
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

void BlockChain::processObject(std::unique_ptr<core::Object> object)
{

}

uint64_t BlockChain::getLastBlockNumber() const
{
    Guard l(x_memoryQueue);
    if (m_memoryQueue.empty()) {
        // read from db

        return 0;
    }

    return m_memoryQueue.back().getBlockNumber();
}

void Dispatch::processMsg(bi::tcp::endpoint const& from, BytesPacket const& msg)
{
    m_chain->processObject(interpretObject(from, msg));
}

std::shared_ptr<core::Transaction> BlockChain::getTransactionFromCache()
{
    std::shared_ptr<core::Transaction> ret = nullptr;
    Guard l(x_transactionsQueue);
    if (!this->transactionsQueue.empty()) {
        ret = this->transactionsQueue.front();
        this->transactionsQueue.pop();
    }

    return ret;
}

std::shared_ptr<core::Block> BlockChain::getBlockFromCache()
{
    std::shared_ptr<core::Block> ret = nullptr;
    Guard l(x_blocksQueue);
    if (!transactionsQueue.empty()) {
        ret = blocksQueue.front();
        blocksQueue.pop();
    }

    return ret;
}

/*
 * 0x01: Transactoon
 * 0x02: BlockHeader
 * 0x03: Block
 * 0x04: Account
 * 0x05: Producer
 * 0x06: SubChain
 *
 */
std::unique_ptr<core::Object> Dispatch::interpretObject(bi::tcp::endpoint const& from, BytesPacket const& msg)
{
    unique_ptr<Object> object;


    switch (msg.getObjectType()) {
        case 0x01: // Transaction
            object.reset(new Transaction(bytesConstRef(&msg.data())));
            break;
        case 0x02: // BlockHeader
            object.reset(new BlockHeader(bytesConstRef(&msg.data())));
            break;
        case 0x03:
            object.reset(new Block(bytesConstRef(&msg.data())));
            break;
        case 0x04:
            object.reset(new Account(bytesConstRef(&msg.data())));
            break;
        case 0x05:
            object.reset(new Producer(bytesConstRef(&msg.data())));
            break;
        case 0x06:
            object.reset(new SubChain(bytesConstRef(&msg.data())));
            break;
        default:
            THROW_GSEXCEPTION("Unknown object type -> interpretObject");
            break;
    }

    return object;
}
} // end namespace