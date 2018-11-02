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

class Block;
extern Block EmptyBlock;

#define BLOCK_HEADER_FIELDS_WITHOUT_SIG (9)
#define BLOCK_HEADER_FIELDS_ALL (9 + 3)

class BlockHeader: public Object {
public:
    BlockHeader();

    BlockHeader(uint64_t number);

    BlockHeader(bytes const& data);

    BlockHeader(bytesConstRef data);

    BlockHeader(BlockHeader const& header);

    BlockHeader& operator=(BlockHeader const& header);

    bool operator==(BlockHeader const& header) const;

    bool operator!=(BlockHeader const& header) const;

    void streamRLP(RLPStream& rlpStream) const;

    void streamRLPContent(RLPStream& rlpStream) const;

    void populate(bytesConstRef data);

    void setChainID(chain::ChainID chainID) { m_chainID = chainID; }

    void setProducer(Address const& producer) { m_producer = producer; }

    void setParentHash(h256 const& parentHash) { m_parentHash = parentHash; }

    void setRoots(trie::TrieType const& mkl, trie::TrieType const& t, trie::TrieType const& r);

    void setNumber(uint64_t number) { m_number = number; }

    void setTimestamp(int64_t timestamp) { m_timestamp = timestamp; }

    void setExtra(bytes const& extra) { m_extra = extra; }

    void setSignature(Signature const& sig) { m_signature = SignatureStruct(sig); }

    void setSignature(SignatureStruct const& sig) { m_signature = sig; }

    void sign(Secret const& priv);

    chain::ChainID getChainID() const { return m_chainID; }

    Address const& getProducer() const { return m_producer; }

    h256 const& getParentHash() const { return m_parentHash; }

    trie::TrieType const& getTrieRoot() const { return m_mklRoot; }

    trie::TrieType const& getTxRoot() const { return m_transactionsRoot; }

    trie::TrieType const& getReceiptRoot() const { return m_receiptRoot; }

    uint64_t getNumber() const { return m_number; }

    int64_t getTimestamp() const { return m_timestamp; }

    bytes const& getExtra() const { return m_extra; }

    h256 const& getHash();

    SignatureStruct const& getSignature() const { return m_signature; }

    void clear();

    // @override
    bytes getKey();

    // @override
    bytes getRLPData();

    // @override
    Object::ObjectType getObjectType() const { return Object::BlockHeaderType; }

private:
    chain::ChainID m_chainID = chain::DEFAULT_GSE_NETWORK;
    Address  m_producer;
    h256 m_parentHash;
    trie::TrieType m_mklRoot;
    trie::TrieType m_transactionsRoot;
    trie::TrieType m_receiptRoot;
    uint64_t m_number;
    int64_t m_timestamp;
    bytes m_extra;
    SignatureStruct m_signature;

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

    void sign(Secret const& priv) { m_blockHeader.sign(priv); }

    Address const& getProducer() const { return m_blockHeader.getProducer(); }

    BlockHeader const& getBlockHeader() const { return m_blockHeader; }

    Transactions const& getTransactions() const { return m_transactions; }

    size_t getTransactionsSize() const { return m_transactions.size(); }

    TransactionReceipts const& getTransactionReceipts() const { return m_transactionReceipts; }

    uint64_t getNumber() const { return m_blockHeader.getNumber(); }

    h256 const& getHash() { return m_blockHeader.getHash(); }

    SignatureStruct const& getSignature() const { return m_blockHeader.getSignature(); }

    // @override
    bytes getKey();

    // @override
    bytes getRLPData();

    // @override
    Object::ObjectType getObjectType() const { return Object::BlockType; }

private:
    BlockHeader m_blockHeader;
    Transactions m_transactions;
    TransactionReceipts m_transactionReceipts;

    //h256 m_hash;
};

using BlockID = h256;

using BlockPtr = std::shared_ptr<Block>;

using Blocks = std::vector<Block>;

extern BlockPtr EmptyBlockPtr;

}  /* namespace end */