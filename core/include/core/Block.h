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

#include <core/Transaction.h>
#include <core/TransactionReceipt.h>
#include <core/Types.h>
#include <core/Address.h>
#include <core/Common.h>
#include <core/RLP.h>
#include <core/Object.h>
#include <chain/Types.h>
#include <trie/Trie.h>

namespace core {

class Repository;

#define BLOCK_HEADER_FIELDS (9)

class BlockHeader: public Object {
    friend class Repository;
public:
    BlockHeader();

    BlockHeader(uint64_t number);

    BlockHeader(bytes const& data);

    BlockHeader(bytes const& data, h256 const& hash);

    BlockHeader(bytesConstRef data);

    BlockHeader(bytesConstRef data, h256 const& hash);

    BlockHeader(BlockHeader const& header);

    BlockHeader& operator=(BlockHeader const& header);

    bool operator==(BlockHeader const& header) const;

    bool operator!=(BlockHeader const& header) const;

    void streamRLP(RLPStream& rlpStream) const;

    void populate(RLP const& rlp);

    void populateFromParent(BlockHeader const& header);

    void setProducer(Address const& producer);

    void setParentHash(h256 const& parentHash);

    void setRoots(trie::TrieType const& mkl, trie::TrieType const& t, trie::TrieType const& r);

    void setNumber(uint64_t number);

    void setTimestamp(int64_t timestamp);

    void setExtra(bytes const& extra);

    void setHash(h256 const& hash);

    Address const& getProducer() const { return m_producer; }

    h256 const& getParentHash() const { return m_parentHash; }

    uint64_t getNumber() const { return m_number; }

    int64_t getTimestamp() const { return m_timestamp; }

    bytes const& getExtra() const { return m_extra; }

    h256 const& getHash() const;

    void clear();

    // @override
    std::string getKey();

    // @override
    std::string getRLPData();

    // @override
    uint8_t getObjectType() const { return 0x02; }

private:
    chain::ChainID m_chainID;
    Address  m_producer;
    h256 m_parentHash;
    trie::TrieType m_mklRoot;
    trie::TrieType m_transactionsRoot;
    trie::TrieType m_receiptRoot;
    uint64_t m_number;
    int64_t m_timestamp;
    bytes m_extra;

    h256 m_hash; // sha3 of the blockheader
};

#define BLOCK_FIELDS 3

class Block: public Object {
public:
    Block();

    Block(BlockHeader const&blockHeader);

    Block(Block const& block);

    Block(bytesConstRef data);

    // ~Block() {} cause signal c + c uncatched exception
    //virtual ~Block();


    void streamRLP(RLPStream& rlpStream) const;

    Block& operator=(Block const& block);

    bool operator==(Block const& block) const;

    bool operator!=(Block const& block) const;

    void setBlockHeader(BlockHeader const& blockHeader);

    void addTransaction(Transaction const& transaction);

    void addTransactionReceipt(TransactionReceipt const& transactionReceipt);

    void setRoots();

    Address const& getProducer() const { return m_blockHeader.getProducer(); }

    BlockHeader const& getBlockHeader() const { return m_blockHeader; }

    Transactions const& getTransactions() const { return m_transactions; }

    size_t getTransactionsSize() const { return m_transactions.size(); }

    TransactionReceipts const& getTransactionReceipts() const { return m_transactionReceipts; }

    uint64_t getNumber() const { return m_blockHeader.getNumber(); }

    h256 const& getHash();

    // @override
    std::string getKey();

    // @override
    std::string getRLPData();

    // @override
    uint8_t getObjectType() const { return 0x03; }

private:
    BlockHeader m_blockHeader;
    Transactions m_transactions;
    TransactionReceipts m_transactionReceipts;

    h256 m_hash;
};

}  /* namespace end */