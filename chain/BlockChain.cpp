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
#include <runtime/correlator/Correlation.h>
#include <runtime/common/Runtime.h>
#include <core/JsonHelper.h>
#include <core/Log.h>
#include <core/Exceptions.h>
#include <crypto/Valid.h>
#include <utils/Utils.h>
#include <config/Constant.h>
#include <config/Argument.h>

using namespace core;
using namespace runtime::storage;
using namespace runtime::common;
using namespace runtime::correlator;
using namespace utils;
using namespace config;

namespace chain {

BlockChain::BlockChain(crypto::GKey const& key, DatabaseController* dbc, BlockChainMessageFace *messageFace, ChainID chainID):
    m_key(key), m_dbc(dbc), m_messageFace(messageFace), m_chainID(chainID), Task("Chain")
{
    m_dispatcher = new Dispatch(this);
    if (m_chainID == GSE_UNKNOWN_NETWORK) {
        throw BlockChainException("Unknown Chain ID:" + toString(GSE_UNKNOWN_NETWORK));
    }
}

BlockChain::~BlockChain()
{
    if (ARGs.m_syncFlag && m_sync) {
        m_sync->stop();
        delete m_sync;
    }

    if (m_dispatcher)
        delete m_dispatcher;
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
    m_rollbackState.m_solidifiable.connect([&](BlockStatePtr bsp) {
        onSolidifiable(bsp);
    });

    if (!m_head) {
        initializeRollbackState();
    }

    ATTRIBUTE_DB_DIRTY = m_dbc->getAttribute(ATTRIBUTE_DB_DIRTY.getKey());
    if (ATTRIBUTE_DB_DIRTY.getValue() == 1) {
        throw BlockChainException("Database corruption, please delete database ans sync again!");
    }

    m_currentActiveProducers.populate(ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST.getData());

    Block solidifyBlock = m_dbc->getBlock(ATTRIBUTE_CURRENT_BLOCK_HEIGHT.getValue());
    m_prevPS.populate(ATTRIBUTE_PREV_PRODUCER_LIST.getData());
    m_currentPS.populate(ATTRIBUTE_CURRENT_PRODUCER_LIST.getData());
    if (m_currentPS.getTimestamp() > solidifyBlock.getTimestamp()) {
        m_currentPS.populate(ATTRIBUTE_PREV_PRODUCER_LIST.getData());
    }

    {
        Guard l{x_latestBlock};
        m_latestBlockNumber = solidifyBlock.getNumber();
        m_latestBlock = solidifyBlock;
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
    m_messageFace->schedule(m_prevPS.getProducers(), m_prevPS.getTimestamp());
    m_messageFace->schedule(m_currentPS.getProducers(), m_currentPS.getTimestamp());
}

 std::shared_ptr<BlockChain::MemoryItem> BlockChain::addMemoryItem(BlockPtr block)
{
    std::shared_ptr<MemoryItem> mItem = std::make_shared<MemoryItem>();;
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
    m_memoryQueue.pop_back();
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

void BlockChain::processTransaction(BlockPtr block, Transaction const& transaction, MemoryItem* mItem)
{
    Runtime runtime(transaction, block, mItem->getRepository());
    runtime.init();
    runtime.excute();
    runtime.finished();
}

void BlockChain::doProcessBlock(BlockPtr block)
{
    bool needCancel;
     std::shared_ptr<MemoryItem> item;
    try {
        updateActiveProducers(block);
        item = addMemoryItem(block);
        needCancel = true;
        /*
        for (auto const& i : block->getTransactions())
            processTransaction(block, i, item);
        */
        CorrelatorFilter correlatorFilter(block, item->getRepository());
        correlatorFilter.run();

        int64_t timestamp = block->getBlockHeader().getTimestamp();
        if (!block->isSyncBlock()) {
            HeaderConfirmation hc(m_chainID, block->getNumber(), block->getHash(), timestamp, m_key.getAddress());
            hc.sign(m_key.getSecret());
            m_rollbackState.add(hc);
            m_messageFace->send(hc);
        }

        unsigned producerPosition = ((timestamp - GENESIS_TIMESTAMP) % TIME_PER_ROUND) / PRODUCER_INTERVAL;
        if ((((timestamp - GENESIS_TIMESTAMP) % SCHEDULE_UPDATE_INTERVAL) / TIME_PER_ROUND >= (SCHEDULE_UPDATE_ROUNDS - 1)) &&
                producerPosition == (NUM_DELEGATED_BLOCKS - 1)) {
            schedule(timestamp);
            m_currentActiveProducers.clear();
        }

        unsigned bonusFactor = (block->getBlockHeader().getTimestamp() - GENESIS_TIMESTAMP) / MILLISECONDS_PER_YEAR;
        uint64_t bonus = BLOCK_BONUS_BASE;
        bonus = bonus >> bonusFactor;
        item->bonus(block->getProducer(), bonus);
        item->setDone();
        eraseSolicitedTx(block);

        {
            Guard l{x_latestBlock};
            m_latestBlockNumber = block->getNumber();
            m_latestBlock = *block;
        }


        BlockHeight height(block->getNumber());
        m_observe.notify(&height);
        m_observe.notify(block.get());
        for (auto& i : block->getTransactions()) {
            m_observe.notify(const_cast<Transaction*>(&i));
        }
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

bool BlockChain::checkBifurcation(BlockPtr block)
{
    auto newItem = m_rollbackState.head();
    if (newItem->getPrev() == m_head->m_blockID) {
        doProcessBlock(block);
    } else if (newItem->getPrev() != m_head->m_blockID) {
        try {
            CWARN << "Switch branch - current number:" << m_head->m_blockNumber << "\t new number:" << newItem->m_blockNumber;
            CWARN << "checkBifurcation - new item's prev hash:" << newItem->getPrev();
            CWARN << "checkBifurcation - current head hash:" << m_head->m_blockID;
            auto branches = m_rollbackState.fetchBranchFrom(newItem->m_blockID, m_head->m_blockID);
            for (auto itr = branches.second.begin(); itr != branches.second.end(); itr++) {
                m_rollbackState.remove((*itr)->m_blockNumber);
                popBlockState();
            }

            if (m_head->m_blockID != branches.second.back()->getPrev()) {
                throw BlockChainException("checkBifurcation - the head is not the second chain's parent block");
            }

        } catch (RollbackStateAncestorException& e) {
            /// Do nothing, not valid block for current chain
            CWARN << e.what();
        } catch (BlockChainException& e) {
            CWARN << e.what();
            CWARN << "***************** GSENetwork Serious Data Error Occur *****************";
            exit(-1);
        } catch (std::exception& e) {
            CWARN << e.what();
        }
    }

    return true;
}

bool BlockChain::processBlock(BlockPtr block)
{
    try {
        int64_t timestamp = block->getBlockHeader().getTimestamp();
        if (block->getProducer() != getExpectedProducer(timestamp)) {
            throw InvalidProducerException("Invalid block producer!");
        }

        /// m_rollbackState.add(*block, m_currentActiveProducers);
        ProducerSnapshot ps = m_messageFace->getProducerSnapshot();
        m_rollbackState.add(*block, ps);

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
    Producers ret;
    std::map<Address, Producer> producerMap;
    if (m_memoryQueue.empty()) {
        CINFO << "getProducerListFromRepo - database";
        producerMap =m_dbc->getProducerList();
        for (auto i : producerMap) {
            ret.push_back(i.second);
        }
        return ret;
    } else {
        CINFO << "getProducerListFromRepo - repository";
        producerMap = m_memoryQueue.back()->getRepository()->getProducerList();
        for (auto i : producerMap) {
            ret.push_back(i.second);
        }
        return ret;
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

    m_messageFace->schedule(m_currentPS.getProducers(), timestamp);

    ProducerSnapshot ps = m_messageFace->getProducerSnapshot();
    m_observe.notify(&ps);

    ATTRIBUTE_PREV_PRODUCER_LIST.setData(m_prevPS.getRLPData());
    m_dbc->putAttribute(ATTRIBUTE_PREV_PRODUCER_LIST);
    ATTRIBUTE_CURRENT_PRODUCER_LIST.setData(m_currentPS.getRLPData());
    m_dbc->putAttribute(ATTRIBUTE_CURRENT_PRODUCER_LIST);
}

Producer BlockChain::getProducer(Address const& address)
{
    return m_dbc->getProducer(address);
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
    m_observe.notify(&m_currentActiveProducers);
}

bool BlockChain::processProducerBlock(BlockPtr block)
{
    if (block->getNumber() <= getSolidifyHeight())
        return false;

    {
        Guard l{x_blockCache};
        m_blockCache.emplace(block);
        return true;
    }
}

uint64_t BlockChain::getLastBlockNumber() const
{
    Guard l(x_latestBlock);
    return m_latestBlockNumber;
}

Block BlockChain::getLastBlock() const
{
    Guard l(x_latestBlock);
    return m_latestBlock;
}

Block BlockChain::getBlockByNumber(uint64_t number)
{
    Block ret;
    {
        Guard l{x_latestBlock};
        if (number > m_latestBlockNumber) {
            return EmptyBlock;
        }
    }

    if (number <= m_rollbackState.getSolidifyNumber()) {
        ret = m_dbc->getBlock(number);
    } else {
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

    ret = m_dbc->getBlock(number);
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

void BlockChain::onSolidifiable(BlockStatePtr bsp)
{
    SolidifyBlockHeight height(bsp->m_blockNumber);
    m_observe.notify(&height);
    {
        Guard l(x_memoryQueue);
         std::shared_ptr<MemoryItem> item = m_memoryQueue.front();
        while (item && bsp->m_blockNumber >= item->getBlockNumber()) {
            if (m_blockChainStatus == Killed)
                return;

            ATTRIBUTE_DB_DIRTY.setValue(1);
            m_dbc->putAttribute(ATTRIBUTE_DB_DIRTY);
            m_memoryQueue.pop_front();
            item->commit();
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
            ATTRIBUTE_DB_DIRTY.setValue(0);
            m_dbc->putAttribute(ATTRIBUTE_DB_DIRTY);

            m_rollbackState.remove(solidifyBSP->m_blockID);
            m_rollbackState.setSolidifyNumber(item->getBlockNumber());

            if (!m_memoryQueue.empty())
                m_memoryQueue.front()->setParentEmpty();
            item = m_memoryQueue.front();
        }
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
    m_sync->stop();
    if (isWorking())
        stopWorking();
    terminate();
}

void BlockChain::doWork()
{
    BlockPtr block = nullptr;
    {
        Guard l{x_blockCache};
        if (!m_blockCache.empty()) {
            auto itr = m_blockCache.get<ByUpBlockNumber>().begin();
            if ((*itr)->getNumber() == (getLastBlockNumber() + 1)) {
                block = *itr;
                m_blockCache.erase(m_blockCache.begin());
            } else if ((*itr)->getNumber() <= getLastBlockNumber()) {
                if ((*itr)->getNumber() <= getSolidifyHeight()) {
                    m_blockCache.erase(m_blockCache.begin());
                } else {
                    block = *itr;
                    m_blockCache.erase(m_blockCache.begin());
                }
            }
        }
    }

    if (block) {
        processBlock(block);
    } else {
        if (isWorking()) {
            sleepMilliseconds(100);
        }
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
    Guard l{x_txCache};
    if (!m_txCache.empty()) {
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
    Guard l(x_blockCache);
    if (!m_blockCache.empty()) {
        auto& item = m_blockCache.get<ByBlockID>();
        auto itr = item.find(block.getHash());
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

    auto item = repository->getBlock(block.getHash());
    if (item == EmptyBlock) {} else {
        return true;
    }

    return false;
}

void BlockChain::preProcessTx(Transaction& tx)
{
    if (tx.getChainID() != m_chainID) {
        CWARN << "The transaction's ChainID is not current ChainID, "
              << "current ChainID is "
              << toString(m_chainID)
              << ", transaction's ChainID is "
              << toString(tx.getChainID());
        throw BlockChainException("The transaction's ChainID is not current ChainID");
    }

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
        {
            Guard l{x_historyTxCache};
            TxID txID = tx.getHash();
            if (m_historyTxCache.isExist(txID)) {
                return false;
            } else {
                m_historyTxCache.push(txID);
            }
        }

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
        {
            Guard l{x_historyTxCache};
            TxID txID = tx.getHash();
            if (m_historyTxCache.isExist(txID)) {
                return;
            } else {
                m_historyTxCache.push(txID);
            }
        }

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
    if (block.getBlockHeader().getChainID() != m_chainID) {
        CWARN << "The block number:" << block.getNumber()
              << " is not current ChainID, current ChainID is "
              << toString(m_chainID)
              << ", block's ChainID is "
              << toString(block.getBlockHeader().getChainID());
        throw BlockChainException("Block's ChainID is not current ChainID");
    }

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
        if (block.getNumber() > getLastBlockNumber() + 1000) {
            return;
        }

        {
            Guard l{x_historyBroadcastBlockCache};
            BlockID blockID = block.getHash();
            if (m_historyBroadcastBlockCache.isExist(blockID)) {
                return;
            } else {
                m_historyBroadcastBlockCache.push(blockID);
            }
        }

        preProcessBlock(from, block);
        {
            Guard l{x_blockCache};
            auto ret = m_blockCache.insert(std::make_shared<Block>(block));
        }
        m_messageFace->broadcast(from, block);
    } catch (BlockChainException& e) {
        // CERROR << "BlockChainException - " << e.what();
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
    } catch (BlockChainException& e) {
        // CERROR << "BlockChainException - " << e.what();
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
                static uint64_t maxPacketSize = 55000;
                uint64_t realBlockSize = 0;
                for (uint64_t i = status.getStart(); i <= status.getEnd(); i++) {
                    Block _block = getBlockByNumber(i);
                    if (_block == EmptyBlock) {
                        break;
                    }

                    realBlockSize += _block.getRLPData().size();
                    if (realBlockSize >= maxPacketSize) {
                        break;
                    }

                    _status.addBlock(_block);
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
            CERROR << "Recv status from " << from << " - Unknown type.";
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
                CERROR << "Unknown packet type - " << chain::pptToString((chain::ProtocolPacketType)type);
                return false;
            }

        } catch (DeserializeException const& e) {
            CERROR << "DeserializeException " <<e.what();
            return false;
        } catch (GSException const& e) {
            CERROR << "GSException " << e.what();
            return false;
        } catch (Exception const& e) {
            CERROR << "Exception " << e.what();
            return false;
        }
    } else {
        CERROR << "Dispatch::processMsg - unknown rlp.";
        return false;
    }
}
}
