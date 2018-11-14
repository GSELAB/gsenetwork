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

#include <vector>

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
        if (rlp.isList() && rlp.itemCount() == BLOCK_HEADER_FIELDS_ALL) {
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
        }
    } catch (GSException const& e) {
        throw DeserializeException("Interpret RLP Block header failed");
    } catch (Exception& e) {
        throw e;
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
              << (bigint) m_number
              << (biging) m_timestamp
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
              << (bigint) m_number
              << (bigint) m_timestamp
              << m_extra;
}

void BlockHeader::populate(bytesConstRef data)
{
    unsigned index = 0;
    try {
        RLP rlp = RLP(data);
        if (rlp.isList() && rlp.itemCount() == BLOCK_HEADER_FIELDS_ALL) {
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
        }
    } catch (GSException const& e) {
        throw DeserializeException("Interpret RLP Block header failed");
    } catch (Exception& e) {
        throw e;
    }
}

void BlockHeader::setRoots(trie::H256 const& mkl, trie::H256 const& t, trie::H256 const& r)
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
            bytesConstRef bytesHeader = rlp[0].data();
            bytesConstRef bytesTransactions = rlp[1].data();
            bytesConstRef bytesReceipts = rlp[2].data();
            m_blockHeader.populate(bytesHeader);
            RLP rlpTx(bytesTransactions);
            if (rlpTx.isList()) {
                for (i = 0; i < rlpTx.itemCount(); i++) {
                    bytesConstRef itemTx = rlpTx[i].data();
                    Transaction tx(itemTx);
                    m_transactions.push_back(tx);
                }
            }

            RLP rlpRp(bytesReceipts);
            if (rlpRp.isList()) {
                for (i = 0; i < rlpRp.itemCount(); i++) {
                    bytesConstRef itemRp = rlpRp[i].data();
                    TransactionReceipt rp(itemRp);
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
    } catch (Exception& e) {
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

trie::H256 Block::getTransactionMerkle()
{
    std::vector<trie::H256> trxDigest;
    trxDigest.reserve(m_transactions.size());

    for (unsigned i = 0; i < m_transactions.size(); i++)
        trxDigest.emplace_back(m_transactions[i].getHash());

    trie::H256 mklRoot = trie::merkle(move(trxDigest));

    return mklRoot;
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

        //RLPStream rlpReceipt;
        //m_transactionReceipts[i].streamRLP(rlpReceipt);
        //receiptsMap.insert(std::make_pair(rlpIndex.out(), rlpReceipt.out()));
    }

    trie::H256 mklRoot = getTransactionMerkle();
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
