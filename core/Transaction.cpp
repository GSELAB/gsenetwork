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

using namespace core;
using namespace crypto;

const uint32_t Transaction::ContractCreation = 0;
const uint32_t Transaction::ContractCall = 1;

Transaction::Transaction()
{
    // TODO: DO NOTHING
}

Transaction::Transaction(chain::ChainID chainID, uint32_t type, Address const& sender, Address const& recipient, bytes const& data, uint64_t value)
{
    m_chainID = chainID;
    m_type = type;
    m_sender = sender;
    m_recipient = recipient;
    m_data = data;
    m_value = value;
}

Transaction::Transaction(bytesConstRef data)
{
    unsigned index = 0;
    RLP const rlp(data);
    try {
        if (!rlp.isList()) {
            BOOST_THROW_EXCEPTION(std::range_error("transaction RLP must be a list"));
            // BOOST_THROW_EXCEPTION(errinfo_comment("transaction RLP must be a list"));
        }

        m_chainID = rlp[index = 0].toInt<chain::ChainID>();
        m_type = rlp[index = 1].toInt<uint32_t>();
        m_sender = rlp[index = 2].toHash<Address>(RLP::VeryStrict) ;
        m_recipient = rlp[index = 3].toHash<Address>(RLP::VeryStrict);
        m_data = rlp[index = 4].toBytes();
        m_value = rlp[index = 5].toInt<uint64_t>();

        int v = rlp[index = 6].toInt<int>();
        h256 r = rlp[index = 7].toInt<u256>();
        h256 s = rlp[index = 8].toInt<u256>();
        m_signature = SignatureStruct(r, s, v);
    } catch (Exception& e) {
        //e << errinfo_name("Invalid transaction format") << BadFieldError(index, toHex(rlp[index].data().toBytes()));
        //throw;
        BOOST_THROW_EXCEPTION(e);
    }
}

Transaction::Transaction(bytes const& data): Transaction(&data)
{
    // TODO: DO NOTHING
}

void Transaction::streamRLP(RLPStream& rlpStream) const
{
    /* // How to make it signature
    if (!m_signature) {
        BOOST_THROW_EXCEPTION(std::range_error("Transaction is unsigned!"));
    }
    */

    rlpStream.appendList(TRANSACTION_FIELDS);
    rlpStream << m_chainID
              << m_type
              << m_sender
              << m_recipient
              << m_data
              << m_value;

    rlpStream << m_signature.v
              << (u256)m_signature.r
              << (u256)m_signature.s;
}

// the sha3 of the transaction include signature
h256 const& Transaction::getHash()
{
    if (m_hash) {
        return m_hash;
    }

    RLPStream rlpStream;
    streamRLP(rlpStream);
    m_hash = sha3(&rlpStream.out());
    return m_hash;
}

void Transaction::sign(Secret const& secret)
{
    auto signature = crypto::sign(secret, getHash());
    SignatureStruct sig = *(SignatureStruct const*)&signature;
    if (sig.isValid()) {
        m_signature = sig;
    }
}

bool Transaction::operator==(Transaction const& transaction) const
{
    return (m_chainID == transaction.getChainID()) &&
        (m_type == transaction.getType()) &&
        (m_sender == transaction.getSender()) &&
        (m_recipient == transaction.getRecipient()) &&
        (m_data == transaction.getData()) &&
        (m_value == transaction.getValue());

        // ???????? <should the signature compare ?> boost::optional<SignatureStruct> m_signature;
}

bool Transaction::operator!=(Transaction const& transaction) const
{
    return !operator==(transaction);
}

void Transaction::setChainID(chain::ChainID chainID)
{
    m_chainID = chainID;
}

void Transaction::setType(uint32_t type)
{
    m_type = type;
}

void Transaction::setSender(Address const& sender)
{
    m_sender = sender;
}

void Transaction::setRecipient(Address const& recipient)
{
    m_recipient = recipient;
}

void Transaction::setData(bytes const& data)
{
    m_data = data;
}

void Transaction::setValue(uint64_t value)
{
    m_value = value;
}

chain::ChainID Transaction::getChainID() const
{
    return m_chainID;
}

uint32_t Transaction::getType() const
{
    return m_type;
}

Address const& Transaction::getSender() const
{
    return m_sender;
}

Address const& Transaction::getRecipient() const
{
    return m_recipient;
}

bytes const& Transaction::getData() const
{
    return m_data;
}

uint64_t Transaction::getValue() const
{
    return m_value;
}

// @override
std::string Transaction::getKey()
{
    return getHash().ref().toString();
}

// @override
std::string Transaction::getRLPData()
{
    core::RLPStream rlpStream;
    streamRLP(rlpStream);
    bytesRef bsr = *(bytesRef*)&rlpStream.out();
    return bsr.toString();
}