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

#include <core/Transaction.h>
#include <core/Exceptions.h>
#include <crypto/SHA3.h>

using namespace crypto;

namespace core {
Transaction EmptyTransaction;

TransactionPtr EmptyTransactionPtr = TransactionPtr();

const uint32_t Transaction::ContractCreation = 0;
const uint32_t Transaction::ContractCall = 1;

Transaction::Transaction()
{
    // TODO: DO NOTHING
}

Transaction::Transaction(Transaction const& transaction)
{
    m_chainID = transaction.getChainID();
    m_type = transaction.getType();
    m_sender = transaction.getSender();
    m_recipient = transaction.getRecipient();
    m_timestamp = transaction.getTimestamp();
    m_data = transaction.getData();
    m_value = transaction.getValue();
    m_signature = transaction.getSignature();
}

Transaction::Transaction(chain::ChainID chainID, uint32_t type, Address const& sender, Address const& recipient,
    uint64_t timestamp, bytes const& data, uint64_t value)
{
    m_chainID = chainID;
    m_type = type;
    m_sender = sender;
    m_recipient = recipient;
    m_timestamp = timestamp;
    m_data = data;
    m_value = value;
}

Transaction::Transaction(bytesConstRef data)
{
    unsigned index = 0;
    RLP const rlp(data);
    try {
        if (!rlp.isList() || rlp.itemCount() != TRANSACTION_FIELDS_ALL) {
            BOOST_THROW_EXCEPTION(std::range_error("transaction RLP must be a list"));
        }

        m_chainID = rlp[index = 0].toInt<chain::ChainID>();
        m_type = rlp[index = 1].toInt<uint32_t>();
        m_sender = rlp[index = 2].toHash<Address>(RLP::VeryStrict) ;
        m_recipient = rlp[index = 3].toHash<Address>(RLP::VeryStrict);
        m_timestamp = rlp[index = 4].toInt<int64_t>();
        m_data = rlp[index = 5].toBytes();
        m_value = rlp[index = 6].toInt<uint64_t>();

        Signature sig = rlp[7].toHash<Signature>(RLP::VeryStrict);
        m_signature = *(SignatureStruct*)&sig;
    } catch (Exception& e) {
        BOOST_THROW_EXCEPTION(e);
    }
}

Transaction::Transaction(bytes const& data): Transaction(&data)
{
    // TODO: DO NOTHING
}

void Transaction::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(TRANSACTION_FIELDS_ALL);
    rlpStream << (bigint) m_chainID
              << m_type
              << m_sender
              << m_recipient
              << (bigint) m_timestamp
              << m_data
              << (bigint) m_value;
    rlpStream << *(Signature*)&m_signature;
}

void Transaction::streamRLPContent(RLPStream& rlpStream) const
{
    rlpStream.appendList(TRANSACTION_FIELDS_WITHOUT_SIG);
    rlpStream << (bigint) m_chainID
              << m_type
              << m_sender
              << m_recipient
              << (bigint) m_timestamp
              << m_data
              << (bigint) m_value;
}

// the sha3 of the transaction not include signature
h256 Transaction::getHash() const
{
    RLPStream rlpStream;
    streamRLPContent(rlpStream);
    return sha3(&rlpStream.out());
}

void Transaction::sign(Secret const& secret)
{
    RLPStream rlpStream;
    streamRLPContent(rlpStream);
    auto signature = crypto::sign(secret, sha3(&rlpStream.out()));
    SignatureStruct sig = *(SignatureStruct const*)&signature;
    if (sig.isValid()) {
        m_signature = sig;
    }
}

Transaction& Transaction::operator=(Transaction const& transaction)
{
    if (&transaction == this) return *this;
    m_chainID = transaction.getChainID();
    m_type = transaction.getType();
    m_sender = transaction.getSender();
    m_recipient = transaction.getRecipient();
    m_timestamp = transaction.getTimestamp();
    m_data = transaction.getData();
    m_value = transaction.getValue();
    m_signature = transaction.getSignature();
    return *this;
}

bool Transaction::operator==(Transaction const& transaction) const
{
    h256 txHash = transaction.getHash();
    return getHash() == txHash;
}

bool Transaction::operator!=(Transaction const& transaction) const
{
    return !operator==(transaction);
}

bytes Transaction::getKey()
{
    return getHash().asBytes();
}

bytes Transaction::getRLPData()
{
    core::RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}
}
