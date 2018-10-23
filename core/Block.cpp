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

#include <core/Block.h>
#include <core/Exceptions.h>
#include <crypto/SHA3.h>
#include <storage/Repository.h>

using namespace core;
using namespace crypto;
using namespace runtime;
using namespace trie;

namespace core {
Block EmptyBlock;

BlockPtr EmptyBlockPtr = BlockPtr();

BlockHeader::BlockHeader()
{
    // TODO: DO NOTHING
}

BlockHeader::BlockHeader(uint64_t number): m_number(number)
{

}

BlockHeader::BlockHeader(bytes const& data): BlockHeader(&data)
{

}

BlockHeader::BlockHeader(bytesConstRef data)
{
    unsigned index = 0;
    try {
        RLP rlp = RLP(data);
        if (!rlp.isList() || rlp.itemCount() != BLOCK_HEADER_FIELDS_ALL) {
            throw DeserializeException("BlockHeader deserialize incorrect list or item count!");
        }
        m_chainID = rlp[index = 0].toInt<chain::ChainID>();
        m_producer = rlp[index = 1].toHash<Address>(RLP::VeryStrict);
        m_parentHash = rlp[index = 2].toHash<h256>(RLP::VeryStrict);
        m_mklRoot = rlp[index = 3].toHash<h256>(RLP::VeryStrict);
        m_transactionsRoot = rlp[index = 4].toHash<h256>(RLP::VeryStrict);
        m_receiptRoot = rlp[index = 5].toHash<h256>(RLP::VeryStrict);
        m_number = rlp[index = 6].toPositiveInt64();
        m_timestamp = rlp[index = 7].toPositiveInt64();
        m_extra = rlp[index = 8].toBytes();

        int v = rlp[index = 9].toInt<int>();
        h256 r = rlp[index = 10].toInt<u256>();
        h256 s = rlp[index = 11].toInt<u256>();
        m_signature = SignatureStruct(r, s, v);
    } catch (DeserializeException& e) {
        throw e;
    } catch (GSException const& e) {
        // e << errinfo_name("Interpret RLP header failed") << BadFieldError(index, toHex(rlp[index - 1].data().toBytes()));
        throw GSException("Interpret RLP Block header failed");
    }
}

BlockHeader::BlockHeader(BlockHeader const& header)
{
    m_chainID = header.getChainID();
    m_producer = header.getProducer();
    m_parentHash = header.getParentHash();
    m_mklRoot = header.getTrieRoot();
    m_transactionsRoot = header.getTxRoot();
    m_receiptRoot = header.getReceiptRoot();
    m_number = header.getNumber();
    m_timestamp = header.getTimestamp();
    m_extra = header.getExtra();
    m_signature = header.getSignature();
}

BlockHeader& BlockHeader::operator=(BlockHeader const& header)
{
    if (&header == this) return *this;
    m_chainID = header.getChainID();
    m_producer = header.getProducer();
    m_parentHash = header.getParentHash();
    m_mklRoot = header.getTrieRoot();
    m_transactionsRoot = header.getTxRoot();
    m_receiptRoot = header.getReceiptRoot();
    m_number = header.getNumber();
    m_timestamp = header.getTimestamp();
    m_extra = header.getExtra();
    m_signature = header.getSignature();

    return *this;
}

bool BlockHeader::operator==(BlockHeader const& header) const
{
    return  (m_chainID == header.getChainID()) &&
            (m_producer == header.getProducer()) &&
            (m_parentHash == header.getParentHash()) &&
            (m_mklRoot == header.getTrieRoot()) &&
            (m_transactionsRoot == header.getTxRoot()) &&
            (m_receiptRoot == header.getReceiptRoot()) &&
            (m_number == header.getNumber()) &&
            (m_timestamp == header.getTimestamp()) &&
            (m_extra == header.getExtra());
}

bool BlockHeader::operator!=(BlockHeader const& header) const
{
    return !operator==(header);
}

void BlockHeader::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(BLOCK_HEADER_FIELDS_ALL);
    rlpStream << m_chainID
              << m_producer
              << m_parentHash
              << m_mklRoot
              << m_transactionsRoot
              << m_receiptRoot
              << m_number
              << m_timestamp
              << m_extra;

    rlpStream << m_signature.v
              << (u256)m_signature.r
              << (u256)m_signature.s;
}

void BlockHeader::streamRLPContent(RLPStream& rlpStream) const
{
    rlpStream.appendList(BLOCK_HEADER_FIELDS_WITHOUT_SIG);
    rlpStream << m_chainID
              << m_producer
              << m_parentHash
              << m_mklRoot
              << m_transactionsRoot
              << m_receiptRoot
              << m_number
              << m_timestamp
              << m_extra;
}

void BlockHeader::populate(bytesConstRef data)
{
    unsigned index = 0;
    try {
        RLP rlp = RLP(data);
        if (!rlp.isList() || rlp.itemCount() != BLOCK_HEADER_FIELDS_ALL) {
            throw DeserializeException("BlockHeader deserialize incorrect list or item count!");
        }
        m_chainID = rlp[index = 0].toInt<chain::ChainID>();
        m_producer = rlp[index = 1].toHash<Address>(RLP::VeryStrict);
        m_parentHash = rlp[index = 2].toHash<h256>(RLP::VeryStrict);
        m_mklRoot = rlp[index = 3].toHash<h256>(RLP::VeryStrict);
        m_transactionsRoot = rlp[index = 4].toHash<h256>(RLP::VeryStrict);
        m_receiptRoot = rlp[index = 5].toHash<h256>(RLP::VeryStrict);
        m_number = rlp[index = 6].toPositiveInt64();
        m_timestamp = rlp[index = 7].toPositiveInt64();
        m_extra = rlp[index = 8].toBytes();

        int v = rlp[index = 9].toInt<int>();
        h256 r = rlp[index = 10].toInt<u256>();
        h256 s = rlp[index = 11].toInt<u256>();
        m_signature = SignatureStruct(r, s, v);
    } catch (DeserializeException& e) {
        throw e;
    } catch (GSException const& e) {
        // e << errinfo_name("Interpret RLP header failed") << BadFieldError(index, toHex(rlp[index - 1].data().toBytes()));
        throw GSException("Interpret RLP Block header failed");
    }
}

void BlockHeader::setRoots(trie::TrieType const& mkl, trie::TrieType const& t, trie::TrieType const& r)
{
    m_mklRoot = mkl;
    m_transactionsRoot = t;
    m_receiptRoot = r;
}

void BlockHeader::sign(Secret const& priv)
{
    Signature sig = crypto::sign(priv, getHash());
    SignatureStruct _sig = *(SignatureStruct*)&sig;
    if (_sig.isValid()) m_signature = _sig;
}

h256 const& BlockHeader::getHash()
{
    RLPStream rlpStream;
    streamRLPContent(rlpStream);
    m_hash = sha3(&rlpStream.out());
    return m_hash;
}

void BlockHeader::clear()
{
    m_chainID = 0;
    m_producer = Address();
    m_parentHash = h256();
    m_mklRoot = h256();
    m_transactionsRoot = h256();
    m_receiptRoot = h256();
    m_number = 0;
    m_timestamp = 0;
    m_extra.clear();
    m_signature = SignatureStruct();
}

// @override
bytes BlockHeader::getKey()
{
    if (m_hash)
        return m_hash.asBytes();

    RLPStream rlpStream;
    streamRLPContent(rlpStream);
    m_hash = sha3(&rlpStream.out());
    return m_hash.asBytes();
}

// @override
bytes BlockHeader::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

/* --- Block ---*/
Block::Block()
{

}

Block::Block(BlockHeader const&blockHeader)
{
    m_blockHeader = blockHeader;
}

Block::Block(Block const& block)
{
    m_blockHeader = block.getBlockHeader();
    m_transactions.assign(block.getTransactions().begin(), block.getTransactions().end());
    m_transactionReceipts.assign(block.getTransactionReceipts().begin(), block.getTransactionReceipts().end()) ;
}

Block::Block(bytesConstRef data)
{
    unsigned i = 0;
    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == BLOCK_FIELDS) {
            // header bytes
            bytes bytesHeader = rlp[0].toBytes();
            bytes bytesTransactions = rlp[1].toBytes();
            bytes bytesReceipts = rlp[2].toBytes();
            m_blockHeader.populate(&bytesHeader);

            RLP rlpTx(bytesTransactions);
            if (rlpTx.isList()) {
                for (i = 0; i < rlpTx.itemCount(); i++) {
                    bytes itemTx = rlpTx[i].toBytes();
                    Transaction tx(&itemTx);
                    m_transactions.push_back(tx);
                }
            }

            RLP rlpRp(bytesReceipts);
            if (rlpRp.isList()) {
                for (i = 0; i < rlpRp.itemCount(); i++) {
                    bytes itemRp = rlpRp[i].toBytes();
                    TransactionReceipt rp(&itemRp);
                    m_transactionReceipts.push_back(rp);
                }
            }
        } else {
            throw DeserializeException("Block deserialize incorrect list or item count!");
        }
    } catch (DeserializeException& e) {
        throw e;
    } catch (GSException& e) {
        throw e;
    }
}

void Block::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(3);

    {
        // block header rlp
        RLPStream rlpStreamBlockHeader;
        m_blockHeader.streamRLP(rlpStreamBlockHeader);

        rlpStream.appendRaw(rlpStreamBlockHeader.out());
    }

    {
        // transactions rlp
        RLPStream rlpStreamTransactions;
        rlpStreamTransactions.appendList(m_transactions.size());
        for (unsigned i = 0, m = m_transactions.size(); i < m; i++) {
            RLPStream rlpStreamTransaction;
            m_transactions[i].streamRLP(rlpStreamTransaction);
            rlpStreamTransactions.appendRaw(rlpStreamTransaction.out());
        }

        rlpStream.appendRaw(rlpStreamTransactions.out());
    }

    {
        RLPStream rlpStreamReceipts;
        rlpStreamReceipts.appendList(m_transactionReceipts.size());
        for (unsigned i = 0, m = m_transactionReceipts.size(); i < m; i++) {
            RLPStream rlpStreamReceipt;
            m_transactionReceipts[i].streamRLP(rlpStreamReceipt);
            rlpStreamReceipts.appendRaw(rlpStreamReceipt.out());
        }

        rlpStream.appendRaw(rlpStreamReceipts.out());
    }
}

Block& Block::operator=(Block const& block)
{
    if (this == &block) return *this;
    m_blockHeader = block.getBlockHeader();
    m_transactions.assign(block.getTransactions().begin(), block.getTransactions().end());
    m_transactionReceipts.assign(block.getTransactionReceipts().begin(), block.getTransactionReceipts().end()) ;
    return *this;
}

bool Block::operator==(Block const& block) const
{
    // Incomplete compare
    return (m_blockHeader == block.getBlockHeader());
}

bool Block::operator!=(Block const& block) const
{
    return !operator==(block);
}

void Block::setBlockHeader(BlockHeader const& blockHeader)
{
    m_blockHeader = blockHeader;
}

void Block::addTransaction(Transaction const& transaction)
{
    // TODO:
    m_transactions.push_back(transaction);
}

void Block::addTransactionReceipt(TransactionReceipt const& transactionReceipt)
{
    // TODO:
    m_transactionReceipts.push_back(transactionReceipt);
}

void Block::setRoots()
{
    BytesMap transactionsMap;
    BytesMap receiptsMap;

    for (unsigned i = 0; i < m_transactions.size(); i++) {
        RLPStream rlpIndex;
        rlpIndex << i;

        RLPStream rlpTransaction;
        m_transactions[i].streamRLP(rlpTransaction);
        transactionsMap.insert(std::make_pair(rlpIndex.out(), rlpTransaction.out()));

        RLPStream rlpReceipt;
        m_transactionReceipts[i].streamRLP(rlpReceipt);
        receiptsMap.insert(std::make_pair(rlpIndex.out(), rlpReceipt.out()));
    }

    trie::TrieType mklRoot;
    {
        // set mklRoot
    }
    m_blockHeader.setRoots(mklRoot, trieHash256(transactionsMap), trieHash256(receiptsMap));
}

// @override
bytes Block::getKey()
{
    return m_blockHeader.getKey();
}

// @override
bytes Block::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

}