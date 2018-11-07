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

#include <chain/Genesis.h>
#include <core/Account.h>
#include <config/Constant.h>
#include <core/JsonHelper.h>

using namespace core;

namespace chain {

void initGenesis(Genesis& genesis, Json::Value const& root)
{
    if (!genesis.m_inited) {
        int64_t genesisTimestamp = root["timestamp"].asInt64();
        genesis.m_producerSnapshot.setTimestamp(genesisTimestamp);
        Json::Value initAccounts = root["init_accounts"];
        for (unsigned i = 0; i < initAccounts.size(); i++) {
            Json::Value item = initAccounts[i];
            Account account(Address(item["address"].asString()), item["balance"].asUInt64(), genesisTimestamp);
            genesis.m_initAccounts.emplace(account.getAddress(), account);
        }

        Json::Value initProducers = root["init_producers"];
        for (unsigned i = 0; i < initProducers.size(); i++) {
            Json::Value item = initProducers[i];
            Producer producer(Address(item["address"].asString()), genesisTimestamp);
            producer.setVotes(item["vote"].asUInt64());
            genesis.m_initProducers.emplace(producer.getAddress(), producer);
            genesis.m_producerSnapshot.addProducer(producer);
        }

        Json::Value genesisBlock = root["genesis_block"];
        ChainID chainID = static_cast<ChainID>(genesisBlock["chainID"].asUInt64());
        Address address(genesisBlock["producer"].asString());
        h256 parentHash(genesisBlock["parentHash"].asString());
        h256 mkl(genesisBlock["merkle"].asString());
        h256 txRoot(genesisBlock["txRoot"].asString());
        h256 receiptRoot(genesisBlock["receiptRoot"].asString());
        uint64_t number = genesisBlock["number"].asUInt64();
        int64_t timestamp = genesisBlock["timestamp"].asInt64();
        std::string extraString = genesisBlock["extra"].asString();
        Signature signature(genesisBlock["signature"].asString());

        BlockHeader blockHeader(number);
        blockHeader.setChainID(chainID);
        blockHeader.setProducer(address);
        blockHeader.setParentHash(parentHash);
        blockHeader.setRoots(mkl, txRoot, receiptRoot);
        blockHeader.setTimestamp(timestamp);
        blockHeader.setExtra(toBytes(extraString));
        blockHeader.setSignature(signature);
        Block block(blockHeader);
        genesis.m_genesisBlock = block;
        /// CINFO << "Genesis block:" << toJson(block).toStyledString();
    }
}
} // end namespace