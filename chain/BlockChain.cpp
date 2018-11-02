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
#include <core/JsonHelper.h>
#include <core/Log.h>
#include <core/Exceptions.h>
#include <crypto/Valid.h>
#include <utils/Utils.h>

using namespace core;
using namespace runtime::storage;
using namespace runtime;
using namespace utils;

namespace chain {

BlockChain::BlockChain(crypto::GKey const& key, DatabaseController* dbc, BlockChainMessageFace *messageFace, ChainID const& chainID):
    m_key(key), m_dbc(dbc), m_messageFace(messageFace), m_chainID(chainID), Task("Chain")
{
    m_dispatcher = new Dispatch(this);
}

BlockChain::~BlockChain()
{
    CINFO << "BlockChain::~BlockChain";
    if (m_dispatcher) delete m_dispatcher;
    stop();
    terminate();
}

void BlockChain::initializeRollbackState()
{
    // m_head = std::make_shared<BlockState>();
}

void BlockChain::init()
{
    CINFO << "Block chain init";
    m_rollbackState.m_irreversible.connect([&](auto bsp) {
        onIrreversible(bsp);
    });

    if (!m_head) {
        initializeRollbackState();
    }
}

BlockChain::MemoryItem* BlockChain::addMemoryItem(std::shared_ptr<Block> block)
{
    CINFO << toJson(*block).toStyledString();
    MemoryItem* mItem = new MemoryItem();
    {
        Guard g(x_memoryQueue);
        std::shared_ptr<runtime::storage::Repository> repository;
        if (m_memoryQueue.empty()) {
            repository = std::make_shared<runtime::storage::Repository>(block, getDBC());
        } else {
            repository = std::make_shared<runtime::storage::Repository>(block, m_memoryQueue.back()->getRepository(), getDBC());
        }

        mItem->setBlockNumber(block->getNumber());
        mItem->setRepository(repository);
        m_memoryQueue.push(mItem);
    }
    return mItem;
}

void BlockChain::cancelMemoryItem()
{
    Guard g(x_memoryQueue);
    m_memoryQueue.pop_back();
}

uint64_t BlockChain::getLastIrreversibleBlockNumber() const
{
    ///return m_rollbackState.m_bftIrreversibleBlockNumber;
    return 0;
}

void BlockChain::commitBlockState(std::shared_ptr<Block> block)
{
    // block->commit();
}

void BlockChain::popBlockState()
{
    auto prev = m_rollbackState.getBlock(m_head->getPrev());
    if (!prev) {
        CERROR << "popBlockState - no prev block exist in rollbackState!";
        throw BlockChainException("popBlockState - no prev block exist in rollbackState!");
    }

    m_head = prev;
    cancelMemoryItem();
}

void BlockChain::doProcessBlock(std::shared_ptr<Block> block)
{
    bool needCancel;
    MemoryItem* item;
    try {
        item = addMemoryItem(block);
        needCancel = true;
        for (auto const& i : block->getTransactions())
            if (!processTransaction(*block, i, item)) {
                // Record the failed
            }

        item->setDone();
    } catch (RollbackStateException& e) {
        if (needCancel) {
            needCancel = false;
            cancelMemoryItem();
        }

    } catch (GSException& e) {
        if (needCancel) {
            needCancel = false;
            cancelMemoryItem();
        }
    }
}

bool BlockChain::processBlock(std::shared_ptr<Block> block)
{
    try {
        for (auto& i : block->getTransactions()) {
            if (!crypto::isValidSig(*const_cast<Transaction*>(&i)))
                throw InvalidTransactionException("Invalid transaction signature!");
        }

        // add block to rollback state
        m_rollbackState.add(*block);
        m_head = m_rollbackState.head();
        checkBifurcation(block);
    } catch (InvalidTransactionException& e) {
        CERROR << "processBlock - " << e.what();
        return false;
    } catch (RollbackStateException& e) {
        CERROR << "RollbackState:" << e.what();
        return false;
    } catch (Exception& e) {
        return false;
    }

    return true;
}

bool BlockChain::processProducerBlock(std::shared_ptr<Block> block)
{
    Guard l{x_blockCache};
    m_blockCache.emplace(block);
    //return processBlock(block);
    return true;
}

bool BlockChain::processTransaction(Block const& block, Transaction const& transaction, MemoryItem* mItem)
{
    try {
        Runtime runtime(transaction, block, mItem->getRepository());
        runtime.init();
        runtime.excute();
        runtime.finished();
    } catch (Exception e) {

    }

    return true;
}

bool BlockChain::processTransaction(Transaction const& transaction, MemoryItem* mItem)
{
    // Runtime runtime();
    return false;
}

bool BlockChain::checkBifurcation(std::shared_ptr<Block> block)
{
    auto newItem = m_rollbackState.head();
    if (newItem->getPrev() == m_head->m_blockID) {
        doProcessBlock(block);
    } else if (newItem->getPrev() != m_head->m_blockID) {
        CINFO << "Switch branch : prevHead(" << m_head->m_blockNumber << ")" << " newHead(" << newItem->m_blockNumber << ")";
        auto branches = m_rollbackState.fetchBranchFrom(newItem->m_blockID, m_head->m_blockID);
        for (auto itr = branches.second.begin(); itr != branches.second.end(); itr++) {
            m_rollbackState.markInCurrentChain(*itr, false);
            popBlockState();
        }

        if (m_head->m_blockID != branches.second.back()->getPrev()) {
            CERROR << "checkBifurcation - error occur when check switch!";
            throw BlockChainException("checkBifurcation - error occur when check switch!");
        }

        for (auto nItr = branches.first.rbegin(); nItr != branches.first.rend(); nItr++) {
            try {
                std::shared_ptr<Block> _block = std::make_shared<Block>((*nItr)->m_block);
                doProcessBlock(_block);
                m_head = *nItr;
                m_rollbackState.markInCurrentChain(m_head, true);
                (*nItr)->m_validated = true;
            } catch (Exception& e) {
                CERROR << "checkBifurcation - error occur when switch!";
                m_rollbackState.setValidity(*nItr, false);
                for (auto _nItr = nItr.base(); _nItr != branches.first.end(); _nItr++) {
                    m_rollbackState.markInCurrentChain(*_nItr, false);
                    popBlockState();
                }

                if (m_head->m_blockID != branches.second.back()->getPrev()) {
                    CERROR << "checkBifurcation - error occur when check switch first!";
                    throw BlockChainException("checkBifurcation - error occur when check switch first!");
                }

                // re-process orginal blocks;
                for (auto rItr = branches.second.rbegin(); rItr != branches.second.rend(); rItr++) {
                    std::shared_ptr<Block> _b = std::make_shared<Block>((*rItr)->m_block);
                    doProcessBlock(_b);
                    m_head = *rItr;
                    m_rollbackState.markInCurrentChain(*rItr, true);
                }

                throw e;
            }
            CINFO << "Switch to new head!";
        }
    }

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



    return m_memoryQueue.back()->getBlockNumber();
}

Block BlockChain::getLastBlock() const
{
    Guard l(x_memoryQueue);
    if (m_memoryQueue.empty()) {
        // read from db

        return EmptyBlock;
    }

    return m_memoryQueue.back()->getBlock();
}

Block BlockChain::getBlockByNumber(uint64_t number)
{
    Guard l(x_memoryQueue);
    // CINFO << "m_memoryQueue size = " << m_memoryQueue.size();
    if (!m_memoryQueue.empty()) {
        auto itemS = m_memoryQueue.front();
        auto itemE = m_memoryQueue.back();
        // CINFO << "start: " << itemS->getBlockNumber() << " end:" << itemE->getBlockNumber();
        if (number >= itemS->getBlockNumber() && number <= itemE->getBlockNumber()) {
            for (Queue_t::iterator iter = m_memoryQueue.begin(); iter != m_memoryQueue.end(); iter++) {
                if ((*iter)->getBlockNumber() == number) {
                    // CINFO << "find " << (*iter)->getBlockNumber() << " --  " << (*iter)->getRepository()->getBlock().getNumber() << " block";
                    return (*iter)->getRepository()->getBlock();
                }
            }
        }
    }

    // search level db
    {

    }

    BlockHeader header(0xFFFFFFFFFFFFFFFF);
    return Block(header);
}

std::shared_ptr<core::Transaction> BlockChain::getTransactionFromCache()
{
    std::shared_ptr<core::Transaction> ret = nullptr;
    Guard l(x_txCache);
    if (!m_txCache.empty()) {
        auto itr = m_txCache.begin();
        ret = *itr;
        m_txCache.erase(itr);
    }

    return ret;
}

std::shared_ptr<core::Block> BlockChain::getBlockFromCache()
{
    std::shared_ptr<core::Block> ret = nullptr;
    Guard l(x_blockCache);
    if (!m_blockCache.empty()) {
        auto itr = m_blockCache.begin();
        ret = *itr;
        m_blockCache.erase(itr);
    }

    return ret;
}

void BlockChain::onIrreversible(BlockStatePtr bsp)
{
    Guard l(x_memoryQueue);
    MemoryItem* item = m_memoryQueue.front();
    while (item && bsp->m_blockNumber >= item->getBlockNumber()) {
        CINFO << "onIrreversible block number:" << bsp->m_blockNumber;
        m_memoryQueue.pop_front();
        item->commit();
        delete item;
        if (!m_memoryQueue.empty())
            m_memoryQueue.front()->setParentEmpty();
        item = m_memoryQueue.front();
    }
}

void BlockChain::start()
{
    startWorking();
    if (isWorking()) return;
    doneWorking();
}

void BlockChain::stop()
{
    if (isWorking()) {
        stopWorking();
    }
}

void BlockChain::doWork()
{
    bool empty;
    BlockPtr block;
    {
        Guard l{x_blockCache};
        if (m_blockCache.empty()) {
            empty = true;
        } else {
            empty = false;
            auto itr = m_blockCache.begin();
            block = *itr;
            m_blockCache.erase(m_blockCache.begin());

        }
    }

    if (empty) {
        sleepMilliseconds(100);
    } else {
        processBlock(block);
    }
}

bool BlockChain::preProcessTx(Transaction& tx)
{
    try {
        std::shared_ptr<runtime::storage::Repository> backItem = nullptr;
        {
            Guard g(x_memoryQueue);
            if (!m_memoryQueue.empty())
                backItem = m_memoryQueue.back()->getRepository();
        }

        std::shared_ptr<runtime::storage::Repository> repository;
        if (backItem) {
            repository = std::make_shared<runtime::storage::Repository>(BlockPtr(), backItem, getDBC());
        } else {
            repository = std::make_shared<runtime::storage::Repository>(BlockPtr(), getDBC());
        }

        Runtime runtime(tx, repository);
        runtime.init();
        runtime.excute();
        runtime.finished();
    } catch (...) {

    }
    return true;
}

bool BlockChain::addRPCTx(Transaction& tx)
{
    if (isExist(tx))
        return false;

    if (!preProcessTx(tx))
        return false;

    {
        Guard l{x_txCache};
        m_txCache.emplace(std::make_shared<Transaction>(tx));
    }

    return true;
}

bool BlockChain::isExist(Transaction& tx)
{
    {
        Guard l{x_txCache};
        auto& item = m_txCache.get<ByTxID>();
        auto itr = item.find(tx.getHash());
        if (itr != item.end())
            return true;
    }

    {
        std::shared_ptr<runtime::storage::Repository> backItem = nullptr;
        {
            Guard g(x_memoryQueue);
            if (!m_memoryQueue.empty())
                backItem = m_memoryQueue.back()->getRepository();
        }

        std::shared_ptr<runtime::storage::Repository> repository;
        if (backItem) {
            repository = std::make_shared<runtime::storage::Repository>(BlockPtr(), backItem, getDBC());
        } else {
            repository = std::make_shared<runtime::storage::Repository>(BlockPtr(), getDBC());
        }

        auto itr = repository->getTransaction(tx.getHash());
        if (itr == EmptyTransaction) {} else {
            return true;
        }
    }

    return false;
}

bool BlockChain::isExist(Block& block)
{
    {
        Guard l(x_blockCache);
        if (!m_blockCache.empty()) {
            auto& item = m_blockCache.get<ByBlockID>();
            auto itr = item.find(block.getHash());
            if (itr != item.end())
                return true;
        }
    }

    {
        std::shared_ptr<runtime::storage::Repository> backItem = nullptr;
        {
            Guard g(x_memoryQueue);
            if (!m_memoryQueue.empty())
                backItem = m_memoryQueue.back()->getRepository();
        }

        std::shared_ptr<runtime::storage::Repository> repository;
        if (backItem) {
            repository = std::make_shared<runtime::storage::Repository>(BlockPtr(), backItem, getDBC());
        } else {
            repository = std::make_shared<runtime::storage::Repository>(BlockPtr(), getDBC());
        }

        auto item = repository->getBlock(block.getHash());
        if (item == EmptyBlock) {} else {
            return true;
        }
    }

    CINFO << "Not find block from memoryDB & levelDB";
    return false;
}

bool BlockChain::preProcessTx(bi::tcp::endpoint const& from, Transaction& tx)
{
    return preProcessTx(tx);
}

bool BlockChain::preProcessBlock(bi::tcp::endpoint const& from, Block& block)
{
    CINFO << "preProcessBlock";
    return true;
}

void BlockChain::processTxMessage(bi::tcp::endpoint const& from, Transaction& tx)
{
    if (!crypto::isValidSig(tx)) {
        CDEBUG << "Recv tx: invalid signature.";
        return;
    }

    if (isExist(tx)) {
        CDEBUG << "Recv tx: repeat tx.";
        return;
    }

    if (!preProcessTx(tx)) {
        CDEBUG << "Recv tx: preProcessTx failed.";
        return;
    }

    {
        Guard l{x_txCache};
        m_txCache.emplace(std::make_shared<Transaction>(tx));
    }

    m_messageFace->broadcast(from, tx);
    CINFO << "Recv tx:" <<  toJson(tx).toStyledString();
}

void BlockChain::processBlockMessage(bi::tcp::endpoint const& from, Block& block)
{
    if (!crypto::isValidSig(block)) {
        CINFO << "Recv block: invalid signature.";
        return;
    }

    if (isExist(block)) {
        CINFO << "Recv block: repeat block.";
        return;
    }

    if (!preProcessBlock(from, block)) {
        CINFO << "Recv block: preProcess block failed.";
        return;
    }

    {
        Guard l{x_blockCache};
        auto ret = m_blockCache.insert(std::make_shared<Block>(block));
        CINFO << "insert status:" << ret.second;
    }

    m_messageFace->broadcast(from, block);
    CINFO << "Recv block:" << toJson(block).toStyledString();
}

void BlockChain::processConfirmationMessage(bi::tcp::endpoint const& from, HeaderConfirmation& confirmation)
{
    try {
        m_rollbackState.add(confirmation);

    } catch (RollbackStateException) {

    }
}

void BlockChain::processStatusMessage(bi::tcp::endpoint const& from, Status& status)
{
    switch (status.getType()) {
        case GetHeight: {
            CINFO << "Recv from " << from <<  " GetHeight.";
            Status _status(ReplyHeight, getLastBlockNumber());
            m_messageFace->send(from, _status);
            break;
        }
        case ReplyHeight: {
            CINFO << "Recv from " << from <<  " ReplyHeight - " << status.getHeight() << ".";
            if (status.getHeight() > getLastBlockNumber()) {
                CINFO << "Need sync from " << from;
                Status _status(SyncBlocks, getLastBlockNumber() + 1, status.getHeight());
                m_messageFace->send(from, _status);
            }
            break;
        }
        case SyncBlocks: {
            CINFO << "Recv from " << from << " SyncBlocks - (" << status.getStart() << ", " << status.getEnd() << ").";
            if (status.getStart() < status.getEnd() && status.getEnd() <= getLastBlockNumber()) {
                Status _status(ReplyBlocks);
                for (uint64_t i = status.getStart(); i <= status.getEnd(); i++) {
                    _status.addBlock(getBlockByNumber(i));
                }
                m_messageFace->send(from, _status);
            }
            break;
        }
        case ReplyBlocks: {
            CINFO << "Recv from " << from << " ReplyBlocks - (" << status.getBlocks().size() << ").";

            break;
        }
        default: {
            CINFO << "Recv from " << from << " Unknown type.";
            break;
        }
    }
}

void Dispatch::processMsg(bi::tcp::endpoint const& from, BytesPacket const& msg)
{
    m_chain->processObject(interpretObject(from, msg));
}

bool Dispatch::processMsg(bi::tcp::endpoint const& from, unsigned type, RLP const& rlp)
{
    if (rlp.isList() && rlp.itemCount() == 1) {
        try {
            bytesConstRef data = rlp[0].data();
            switch (type) {
            case chain::StatusPacket: {
                Status status(data);
                m_chain->processStatusMessage(from, status);
                return true;
            }
            case chain::TransactionPacket: {
                Transaction tx(data);
                m_chain->processTxMessage(from, tx);
                return true;
            }
            case chain::BlockPacket: {
                Block block(data);
                m_chain->processBlockMessage(from, block);
                return true;
            }
            case chain::ConfirmationPacket: {
                HeaderConfirmation confirmation(data);
                m_chain->processConfirmationMessage(from, confirmation);
                return true;
            }
            case chain::TransactionsPacket: {
                return true;
            }
            case chain::BlockBodiesPacket: {
                return true;
            }
            case chain::NewBlockPacket: {
                return true;
            }
            default:
                CINFO << "Unknown packet type - " << chain::pptToString((chain::ProtocolPacketType)type);
                return false;
            }

        } catch (DeserializeException const& e) {
            CINFO << "DeserializeException " <<e.what();
            return false;
        } catch (GSException const& e) {
            CINFO << "GSException " << e.what();
            return false;
        } catch (Exception const& e) {
            CINFO << "Exception " << e.what();
            return false;
        }
    } else {
        CINFO << "Dispatch::processMsg - unknown rlp.";
        return false;
    }

}

bool Dispatch::processMsg(bi::tcp::endpoint const& from, unsigned type, bytes const& data)
{

    return false;
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
    switch (msg.cap()) {
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