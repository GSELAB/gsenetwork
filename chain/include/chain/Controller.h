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
#include <rpc/RpcService.h>

using namespace core;
using namespace net;

namespace chain {

static std::string ChainVersion("version 1.0");

// @thread safe
class Controller: public producer::ProcuderEventHandleFace, public rpc::WebSocketEventHandlerFace,
    public BlockChainMessageFace {
public:
    Controller() = default;

    virtual ~Controller() {}

    void init(crypto::GKey const& key, ChainID chainID);

    void exit();

    chain::ChainID getChainID() const;

    void setChainID(ChainID chainID);

    database::DatabaseController* getDBC() const { return m_dbc; }

public: // RPC Handle
    virtual string const& getVersion() const override { return ChainVersion; }

    virtual uint64_t getBlockNumberRef() const override { return getLastBlockNumber(); }

    virtual void broadcast(Transaction& tx) override;

    virtual Block getBlockByNumber(uint64_t number) override;

    virtual Transaction getTransaction(TxID const& txID) override { return m_chain->getTx(txID); }

    virtual Producer getProducer(Address const& address) override { return m_chain->getProducer(address); }

    virtual Account getAccount(Address const& address) override { return m_chain->getAccount(address); }

    virtual uint64_t getBalance(Address const& address) override { return m_chain->getBalance(address); }

    virtual uint64_t getHeight() const override { return m_chain->getHeight(); }

    virtual uint64_t getSolidifyHeight() const override { return m_chain->getSolidifyHeight(); }

    virtual Producers getCurrentProducerList() const override { return m_producerServer->getCurrentProducerList(); }

    void registerObserver(Observer<Object*> const& observer) override { m_chain->registerObserver(observer); }

public: // Producer Handle
    virtual void broadcast(std::shared_ptr<Block> block) override;

    virtual void processProducerEvent() override;

    virtual uint64_t getLastBlockNumber() const override { return m_chain->getLastBlockNumber(); }

    virtual Block getLastBlock() const override { return m_chain->getLastBlock(); }

    virtual std::shared_ptr<core::Transaction> getTransactionFromCache() override { return m_chain->getTransactionFromCache(); }

    virtual std::shared_ptr<core::Block> getBlockFromCache() override { return m_chain->getBlockFromCache(); }

    virtual BlockChainStatus getBlockChainStatus() const override { return m_chain->getBlockChainStatus(); };

    virtual bool checkTransactionNotExisted(TxID const& txID) override { return (m_chain->getTx(txID) == EmptyTransaction); }

public: // used by block chain
    virtual void broadcast(boost::asio::ip::tcp::endpoint const& from, Block& block) override;

    virtual void broadcast(boost::asio::ip::tcp::endpoint const& from, BlockPtr block) override;

    virtual void broadcast(boost::asio::ip::tcp::endpoint const& from, Transaction& tx) override;

    virtual void broadcast(boost::asio::ip::tcp::endpoint const& from, TransactionPtr tx) override;

    virtual void broadcast(boost::asio::ip::tcp::endpoint const& from, HeaderConfirmation& hc) override;

    virtual void broadcast(boost::asio::ip::tcp::endpoint const& from, HeaderConfirmationPtr hcp) override;

    virtual void send(HeaderConfirmation& hc) override;

    virtual void send(HeaderConfirmationPtr hcp) override;

    virtual void send(boost::asio::ip::tcp::endpoint const& to, Status& status) override;

    virtual void send(boost::asio::ip::tcp::endpoint const& to, StatusPtr status) override;

    virtual void send(boost::asio::ip::tcp::endpoint const& to, BlockState& bs) override;

    virtual void send(boost::asio::ip::tcp::endpoint const& to, BlockStatePtr bsp) override;

    virtual void schedule(ProducersConstRef producerList, int64_t timestamp) override { m_producerServer->schedule(producerList, timestamp); }

    virtual Address getProducerAddress(unsigned idx) const override { return m_producerServer->getProducerAddress(idx); }

    virtual ProducersConstRef getSortedProducerList() const override { return m_producerServer->getSchedule(); }

    virtual ProducerSnapshot getProducerSnapshot() const override { return m_producerServer->getProducerSnapshot(); }

private:
    ChainID m_chainID = GSE_UNKNOWN_NETWORK;
    crypto::GKey m_key;

    BlockChain* m_chain = nullptr;
    net::NetController* m_net = nullptr;
    database::DatabaseController* m_dbc = nullptr;
    producer::ProducerServer* m_producerServer = nullptr;
    rpc::RpcService *m_rpcServer = nullptr;
};

extern Controller controller;
} /* end namespace */
