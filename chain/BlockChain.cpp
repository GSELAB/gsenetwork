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
#include <config/Constant.h>
#include <config/Argument.h>

using namespace core;
using namespace runtime::storage;
using namespace runtime;
using namespace utils;
using namespace config;

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
    {
        Guard l{x_memoryQueue};
        while (!m_memoryQueue.empty()) {
            auto i = m_memoryQueue.back();
            m_memoryQueue.pop_back();
            delete i;
        }
    }

    if (ARGs.m_syncFlag && m_sync) {
        m_sync->stop();
        delete m_sync;
    }
}

void BlockChain::initializeRollbackState()
{
    Block block = m_dbc->getBlock(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue());

    m_head = std::make_shared<BlockState>(block);
    m_head->m_bftSolidifyBlockNumber = block.getNumber();
    m_rollbackState.set(m_head);
}

void BlockChain::init()
{
    m_rollbackState.m_irreversible.connect([&](BlockStatePtr bsp) {
        onIrreversible(bsp);
    });

    if (!m_head) {
        initializeRollbackState();
    }

    m_currentActiveProducers.populate(ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST.getData());

    Block solidifyBlock = m_dbc->getBlock(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue());
    m_prevPS.populate(ATTRIBUTE_PREV_PRODUCER_LIST.getData());
    m_currentPS.populate(ATTRIBUTE_CURRENT_PRODUCER_LIST.getData());
    if (m_currentPS.getTimestamp() > solidifyBlock.getTimestamp()) {
        m_currentPS.populate(ATTRIBUTE_PREV_PRODUCER_LIST.getData());
    }

    if (ARGs.m_syncFlag) {
        m_blockChainStatus = SyncStatus;
        m_sync = new Sync(this);
        m_sync->start();
    } else {
        m_blockChainStatus = ProducerStatus;
    }
}

void BlockChain::pushSchedule()
{
    m_messageFace->schedule(m_prevPS.getProducers());
    m_messageFace->schedule(m_currentPS.getProducers());
}

BlockChain::MemoryItem* BlockChain::addMemoryItem(BlockPtr block)
{
    MemoryItem* mItem = new MemoryItem();
    Block repoBlock = *block;
    {
        Guard g(x_memoryQueue);
        std::shared_ptr<runtime::storage::Repository> repository;
        if (m_memoryQueue.empty()) {
            repository = std::make_shared<runtime::storage::Repository>(repoBlock, getDBC());
        } else {
            repository = std::make_shared<runtime::storage::Repository>(repoBlock, m_memoryQueue.back()->getRepository(), getDBC());
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
    auto i = m_memoryQueue.back();
    m_memoryQueue.pop_back();
    delete i;
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

void BlockChain::eraseSolicitedTx(BlockPtr block)
{
    if (block->getTransactionsSize() == 0)
        return;

    {
        Guard l{x_txCache};
        auto& idx = m_txCache.get<ByTxID>();
        for (auto i : block->getTransactions()) {
            auto itr = idx.find(i.getHash());
            if (itr != idx.end())
                idx.erase(itr);
        }

    }
}

void BlockChain::processTransaction(Block const& block, Transaction const& transaction, MemoryItem* mItem)
{
    Runtime runtime(transaction, block, mItem->getRepository());
    runtime.init();
    runtime.excute();
    runtime.finished();
}

void BlockChain::doProcessBlock(BlockPtr block)
{
    bool needCancel;
    MemoryItem* item;
    CINFO << "Process block number:" << block->getNumber() << "\ttx.size:" << block->getTransactionsSize();
    try {
        item = addMemoryItem(block);
        needCancel = true;
        for (auto const& i : block->getTransactions())
            processTransaction(*block, i, item);

        int64_t timestamp = block->getBlockHeader().getTimestamp();
        if (!block->isSyncBlock()) {
            HeaderConfirmation hc(m_chainID, block->getNumber(), block->getHash(), timestamp, m_key.getAddress());
            hc.sign(m_key.getSecret());
            m_rollbackState.add(hc);
            m_messageFace->send(hc);
        }

        if (((timestamp - GENESIS_TIMESTAMP) % (SCHEDULE_UPDATE_INTERVAL)) / (TIME_PER_ROUND) >= (SCHEDULE_UPDATE_ROUNDS - 1)) {
            schedule(timestamp);
            m_currentActiveProducers.clear();
        }

        unsigned bonusFactor = (block->getBlockHeader().getTimestamp() - GENESIS_TIMESTAMP) / MILLISECONDS_PER_YEAR;
        uint64_t bonus = BLOCK_BONUS_BASE;
        bonus = bonus >> bonusFactor;
        item->bonus(block->getProducer(), bonus);
        item->setDone();
        eraseSolicitedTx(block);
    } catch (RepositoryException& e) {
        CERROR << "RepositoryException - " << e.what();
        if (needCancel) {
            needCancel = false;
            cancelMemoryItem();
        }
    } catch (RollbackStateException& e) {
        CERROR << "RollbackStateException - " << e.what();
        if (needCancel) {
            needCancel = false;
            cancelMemoryItem();
        }
    } catch (GSException& e) {
        CERROR << "GSException - " << e.what();
        if (needCancel) {
            needCancel = false;
            cancelMemoryItem();
        }
    }
}

bool BlockChain::processBlock(BlockPtr block)
{
    try {
        int64_t timestamp = block->getBlockHeader().getTimestamp();
        if (block->getProducer() != getExpectedProducer(timestamp)) {
            throw InvalidProducerException("Invalid block producer!");
        }

        updateActiveProducers(block);
        m_rollbackState.add(*block, m_currentActiveProducers);
        checkBifurcation(block);
        m_head = m_rollbackState.head();
    } catch (InvalidTransactionException& e) {
        CERROR << "processBlock - " << e.what();
        return false;
    } catch (InvalidProducerException& e) {
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

Producers BlockChain::getProducerListFromRepo() const
{
    Guard l(x_memoryQueue);
    if (m_memoryQueue.empty()) {
        return m_dbc->getProducerList();
    } else {
        return m_memoryQueue.back()->getRepository()->getProducerList();
    }
}

void BlockChain::schedule(int64_t timestamp) {

    Producers updatedSchedule = getProducerListFromRepo();

    m_prevPS = m_currentPS;
    m_currentPS.clear();
    for (auto i : updatedSchedule) {
        m_currentPS.addProducer(i);
    }
    m_currentPS.setTimestamp(timestamp);

    m_messageFace->schedule(m_currentPS.getProducers());

    ATTRIBUTE_PREV_PRODUCER_LIST.setData(m_prevPS.getRLPData());
    m_dbc->putAttribute(ATTRIBUTE_PREV_PRODUCER_LIST);
    ATTRIBUTE_CURRENT_PRODUCER_LIST.setData(m_currentPS.getRLPData());
    m_dbc->putAttribute(ATTRIBUTE_CURRENT_PRODUCER_LIST);
}

Producer BlockChain::getProducer(Address const& address)
{
    Guard g(x_memoryQueue);
    if (m_memoryQueue.empty()) {
        return getDBC()->getProducer(address);
    } else {
        return m_memoryQueue.back()->getRepository()->getProducer(address);
    }
}

Address BlockChain::getExpectedProducer(int64_t timestamp) const
{
    unsigned producerPosition = ((timestamp - GENESIS_TIMESTAMP) %
                (TIME_PER_ROUND)) / (PRODUCER_INTERVAL);

    return m_messageFace->getProducerAddress(producerPosition);
}

void BlockChain::updateActiveProducers(BlockPtr block)
{
    int64_t currentTimestamp = block->getBlockHeader().getTimestamp();
    unsigned currentProducerIndex = ((currentTimestamp - GENESIS_TIMESTAMP) %
                (TIME_PER_ROUND)) / (PRODUCER_INTERVAL);

    Block prevBlock(getBlockByNumber(block->getBlockHeader().getNumber() - 1));
    int64_t prevTimestamp = prevBlock.getBlockHeader().getTimestamp();
    unsigned prevProducerIndex = ((prevTimestamp - GENESIS_TIMESTAMP) %
                (TIME_PER_ROUND)) / (PRODUCER_INTERVAL);

    m_currentActiveProducers.setTimestamp(currentTimestamp);

    m_currentActiveProducers.addProducer(getProducer(block->getBlockHeader().getProducer()));

    ProducersConstRef producerList = m_messageFace->getSortedProducerList();

    if (prevProducerIndex < currentProducerIndex) {
        for (int i = prevProducerIndex + 1; i < currentProducerIndex; ++i)
            m_currentActiveProducers.deleteProducer(producerList[i]);
    } else if (prevProducerIndex == currentProducerIndex) {
        for (int i = 0; i < producerList.size(); ++i) {
            if (i == currentProducerIndex)
                continue;
            m_currentActiveProducers.deleteProducer(producerList[i]);
        }
    } else {
        for (int i = 0; i < currentProducerIndex; ++i)
            m_currentActiveProducers.deleteProducer(producerList[i]);

        for (int j = prevProducerIndex + 1; j < producerList.size(); ++j)
            m_currentActiveProducers.deleteProducer(producerList[j]);
    }
}

bool BlockChain::processProducerBlock(BlockPtr block)
{
    Guard l{x_blockCache};
    m_blockCache.emplace(block);
    return true;
}

bool BlockChain::checkBifurcation(BlockPtr block)
{
    auto newItem = m_rollbackState.head();
    // CINFO << "checkBifurcation - prev hash:" << newItem->getPrev();
    // CINFO << "checkBifurcation - head hash:" << m_head->m_blockID;
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

uint64_t BlockChain::getLastBlockNumber() const
{
    Guard l(x_memoryQueue);
    if (m_memoryQueue.empty()) {
        return ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue();
    }

    return m_memoryQueue.back()->getBlockNumber();
}

Block BlockChain::getLastBlock() const
{
    Guard l(x_memoryQueue);
    if (m_memoryQueue.empty())
        return m_dbc->getBlock(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue());
    return m_memoryQueue.back()->getBlock();
}

Block BlockChain::getBlockByNumber(uint64_t number)
{
    {
        Guard l(x_memoryQueue);
        if (!m_memoryQueue.empty()) {
            auto itemS = m_memoryQueue.front();
            auto itemE = m_memoryQueue.back();
            if (number >= itemS->getBlockNumber() && number <= itemE->getBlockNumber()) {
                for (Queue_t::iterator iter = m_memoryQueue.begin(); iter != m_memoryQueue.end(); iter++)
                    if ((*iter)->getBlockNumber() == number)
                        return (*iter)->getRepository()->getBlock();
            }
        }
    }

    Block ret = m_dbc->getBlock(number);
    return ret;
}

BlockState BlockChain::getBlockStateByNumber(uint64_t number)
{
    if (number >= m_rollbackState.getSolidifyNumber()) {
        auto bsp = m_rollbackState.getBlock(number);
        if (bsp != EmptyBlockStatePtr)
            return *bsp;
    }

    return m_dbc->getBlockState(number);
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

BlockPtr BlockChain::getBlockFromCache()
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
        m_memoryQueue.pop_front();
        item->commit();
        CINFO << "onIrreversible block number:" << item->getBlockNumber();
        BlockStatePtr solidifyBSP = m_rollbackState.getBlock(item->getBlockNumber());
        if (solidifyBSP == EmptyBlockStatePtr) {
            throw BlockChainException("onIrreversible - block state not found");
        }

        ATTRIBUTE_CURRENT_BLOCK_HEIGHT.setValue(solidifyBSP->m_blockNumber);
        m_dbc->putAttribute(ATTRIBUTE_CURRENT_BLOCK_HEIGHT);
        ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST.setData(solidifyBSP->m_activeProucers.getRLPData());
        m_dbc->putAttribute(ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST);
        BlockState solidifyBS = *solidifyBSP;
        m_dbc->put(solidifyBS);
        m_rollbackState.remove(solidifyBSP->m_blockID);
        m_rollbackState.setSolidifyNumber(item->getBlockNumber());
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
    BlockPtr block = nullptr;
    {
        Guard l{x_blockCache};
        if (m_blockCache.empty()) {
            empty = true;
        } else {
            empty = false;
            auto itr = m_blockCache.get<ByUpBlockNumber>().begin();
            // CINFO << "BlockChain - lastNumber:" << getLastBlockNumber() << "  cacheNumber:" << (*itr)->getNumber();
            if ((*itr)->getNumber() > (getLastBlockNumber() + 1)) {

            } else if ((*itr)->getNumber() == (getLastBlockNumber() + 1)) {
                block = *itr;
                m_blockCache.erase(m_blockCache.begin());
            } else {
                block = *itr;
                m_blockCache.erase(m_blockCache.begin());
            }
        }
    }

    if (empty) {
        sleepMilliseconds(100);
    } else {
        if (block)
            processBlock(block);
        else
            sleepMilliseconds(100);
    }
}

Transaction BlockChain::getTx(TxID const& txID)
{
    Guard g(x_memoryQueue);
    if (m_memoryQueue.empty()) {
        return getDBC()->getTransaction(txID);
    } else {
        return m_memoryQueue.back()->getRepository()->getTransaction(txID);
    }
}

Account BlockChain::getAccount(Address const& address)
{
    Guard g(x_memoryQueue);
    if (m_memoryQueue.empty()) {
        return getDBC()->getAccount(address);
    } else {
        return m_memoryQueue.back()->getRepository()->getAccount(address);
    }
}

uint64_t BlockChain::getBalance(Address const& address)
{
    Account account = getAccount(address);
    return account.getBalance();
}

uint64_t BlockChain::getHeight() const
{
    return getLastBlockNumber();
}

uint64_t BlockChain::getSolidifyHeight() const
{
    return ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue();
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

    std::shared_ptr<runtime::storage::Repository> backItem = nullptr;
    {
        Guard g(x_memoryQueue);
        if (!m_memoryQueue.empty())
            backItem = m_memoryQueue.back()->getRepository();
    }

    std::shared_ptr<runtime::storage::Repository> repository;
    if (backItem) {
        repository = std::make_shared<runtime::storage::Repository>(EmptyBlock, backItem, getDBC());
    } else {
        repository = std::make_shared<runtime::storage::Repository>(EmptyBlock, getDBC());
    }

    auto itr = repository->getTransaction(tx.getHash());
    if (itr == EmptyTransaction) {} else {
        return true;
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

    std::shared_ptr<runtime::storage::Repository> backItem = nullptr;
    {
        Guard g(x_memoryQueue);
        if (!m_memoryQueue.empty())
            backItem = m_memoryQueue.back()->getRepository();
    }

    std::shared_ptr<runtime::storage::Repository> repository;
    if (backItem) {
        repository = std::make_shared<runtime::storage::Repository>(EmptyBlock, backItem, getDBC());
    } else {
        repository = std::make_shared<runtime::storage::Repository>(EmptyBlock, getDBC());
    }

    auto item = repository->getBlock(block.getHash());
    if (item == EmptyBlock) {} else {
        return true;
    }

    return false;
}

void BlockChain::preProcessTx(Transaction& tx)
{
    if (!crypto::validSignature(tx)) {
        throw BlockChainException("Transaction(" + toString(tx.getHash()) + ") is not valid signature." );
    }

    if (isExist(tx)) {
        throw BlockChainException("Transaction(" + toString(tx.getHash()) + ") has exist.");
    }

    std::shared_ptr<runtime::storage::Repository> backItem = nullptr;
    {
        Guard g(x_memoryQueue);
        if (!m_memoryQueue.empty())
            backItem = m_memoryQueue.back()->getRepository();
    }

    std::shared_ptr<runtime::storage::Repository> repository;
    if (backItem) {
        repository = std::make_shared<runtime::storage::Repository>(EmptyBlock, backItem, getDBC());
    } else {
        repository = std::make_shared<runtime::storage::Repository>(EmptyBlock, getDBC());
    }

    Runtime runtime(tx, repository);
    runtime.init();
    runtime.excute();
    runtime.finished();
}

bool BlockChain::addRPCTx(Transaction& tx)
{
    try {
        preProcessTx(tx);
        {
            Guard l{x_txCache};
            m_txCache.insert(std::make_shared<Transaction>(tx));
        }
    } catch (BlockChainException& e) {
        CERROR << "BlockChainException - " << e.what();
        return false;
    } catch (RepositoryException& e) {
        CERROR << "RepositoryException - " << e.what();
        return false;
    } catch (Exception& e) {
        CERROR << "Exception - " << e.what();
        return false;
    } catch (std::exception& e) {
        CERROR << "exception - " << e.what();
        return false;
    }

    return true;
}

void BlockChain::processTxMessage(bi::tcp::endpoint const& from, Transaction& tx)
{
    try {
        preProcessTx(tx);
        {
            Guard l{x_txCache};
            m_txCache.insert(std::make_shared<Transaction>(tx));
        }
        m_messageFace->broadcast(from, tx);
    } catch (BlockChainException& e) {
        CERROR << "BlockChainException - " << e.what();
    } catch (RepositoryException& e) {
        CERROR << "RepositoryException - " << e.what();
    } catch (Exception& e) {
        CERROR << "Exception - " << e.what();
    }
}

void BlockChain::preProcessBlock(bi::tcp::endpoint const& from, Block& block)
{
    if (!crypto::validSignature(block)) {
        throw BlockChainException("Block(number:" + toString(block.getNumber()) + " hash:" + toString(block.getHash()) + ") is not valid signature." );
    }

    if (isExist(block)) {
        throw BlockChainException("Block(number:" + toString(block.getNumber()) + " hash:" + toString(block.getHash()) + ") has exist.");
    }

    std::shared_ptr<runtime::storage::Repository> backItem = nullptr;
    {
        Guard g(x_memoryQueue);
        if (!m_memoryQueue.empty())
            backItem = m_memoryQueue.back()->getRepository();
    }

    std::shared_ptr<runtime::storage::Repository> repository;
    if (backItem) {
        repository = std::make_shared<runtime::storage::Repository>(EmptyBlock, backItem, getDBC());
    } else {
        repository = std::make_shared<runtime::storage::Repository>(EmptyBlock, getDBC());
    }

    for (auto i : block.getTransactions()) {
        Runtime runtime(i, repository);
        runtime.init();
        runtime.excute();
        runtime.finished();
    }
}

void BlockChain::processBlockMessage(bi::tcp::endpoint const& from, Block& block)
{
    try {
        preProcessBlock(from, block);
        {
            Guard l{x_blockCache};
            auto ret = m_blockCache.insert(std::make_shared<Block>(block));
        }
        m_messageFace->broadcast(from, block);
        CINFO << "Recv broadcast block number:" << block.getNumber(); // toJson(block).toStyledString();
    } catch (BlockChainException& e) {
        CERROR << "BlockChainException - " << e.what();
    } catch (RepositoryException& e) {
        CERROR << "RepositoryException - " << e.what();
    } catch (Exception& e) {
        CERROR << "Exception - " << e.what();
    }
}

void BlockChain::processSyncBlockMessage(bi::tcp::endpoint const& from, Block& block)
{
    try {
        preProcessBlock(from, block);
        {
            Guard l{x_blockCache};
            auto ret = m_blockCache.insert(std::make_shared<Block>(block));
        }
        CINFO << "Recv sync block number:" << block.getNumber(); // toJson(block).toStyledString();
    } catch (BlockChainException& e) {
        CERROR << "BlockChainException - " << e.what();
    } catch (RepositoryException& e) {
        CERROR << "RepositoryException - " << e.what();
    } catch (Exception& e) {
        CERROR << "Exception - " << e.what();
    }
}

void BlockChain::processConfirmationMessage(bi::tcp::endpoint const& from, HeaderConfirmation& confirmation)
{
    try {
        m_rollbackState.add(confirmation);
    } catch (RollbackStateException& e) {
        CERROR << "RollbackStateException - " << e.what();
    }
}

void BlockChain::processStatusMessage(bi::tcp::endpoint const& from, Status& status)
{
    switch (status.getType()) {
        case GetHeight: {
            Status _status(ReplyHeight, getLastBlockNumber());
            m_messageFace->send(from, _status);
            break;
        }
        case ReplyHeight: {
            if (status.getHeight() > getLastBlockNumber()) {
                m_sync->update(from, status.getHeight());
            }
            break;
        }
        case SyncBlocks: {
            if (status.getStart() < status.getEnd() && status.getEnd() <= getLastBlockNumber()) {
                Status _status(ReplyBlocks);
                BlockState _bs(EmptyBlockState);
                for (uint64_t i = status.getStart(); i <= status.getEnd(); i++) {
                    _status.addBlock(getBlockByNumber(i));
                    BlockState bs;
                    if (i < 11) {
                        bs =  getBlockStateByNumber(i);
                    } else {
                        bs =  getBlockStateByNumber(i - 10);
                    }
                    if (bs != EmptyBlockState && bs.isSolidified()) {
                        _bs = bs;
                    }
                }
                m_messageFace->send(from, _status);
                if (_bs != EmptyBlockState) {
                    m_messageFace->send(from, _bs);
                }
            }
            break;
        }
        case ReplyBlocks: {
            for (auto i : status.getBlocks()) {
                i.setSyncBlock();
                processSyncBlockMessage(from, i);
            }
            break;
        }
        default: {
            CINFO << "Recv status from " << from << " - Unknown type.";
            break;
        }
    }
}

void BlockChain::processBlockStateMessage(bi::tcp::endpoint const& from, BlockState const& bs)
{
    m_rollbackState.addSyncBlockState(bs);
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
            case chain::BlockStatePacket: {
                BlockState bs(data);
                m_chain->processBlockStateMessage(from, bs);
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
}
