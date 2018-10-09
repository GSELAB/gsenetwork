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

#include <core/TransactionReceipt.h>
#include <core/RLP.h>

namespace core {

TransactionReceipt::TransactionReceipt(bytesConstRef data): m_isCode(true)
{

    try {
        RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == TRANSACTION_RECEIPT_FIELDS) {
            if (rlp[0].size() == sizeof(uint32_t)) {
                m_receiptCode = rlp[0].toInt<uint32_t>();
            } else {
                m_stateRoot = rlp[1].toHash<h256>();
                m_isCode = false;
            }
        } else {
            BOOST_THROW_EXCEPTION(std::range_error("Invalid transaction receipt format!"));
        }
    } catch(Exception e) {
        BOOST_THROW_EXCEPTION(e);
    }
}

TransactionReceipt::TransactionReceipt(uint32_t status): m_isCode(true)
{
    m_receiptCode = status;
}

TransactionReceipt::TransactionReceipt(h256 const& root): m_isCode(false)
{
    m_stateRoot = root;
}

void TransactionReceipt::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(TRANSACTION_RECEIPT_FIELDS);
    if (m_isCode) {
        rlpStream << m_receiptCode;
    } else {
        rlpStream << m_stateRoot;
    }
}

bytes TransactionReceipt::rlpBytes() const
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

bool TransactionReceipt::hasReceiptCode() const
{
    return m_isCode;
}

uint32_t TransactionReceipt::getReceiptCode() const
{
    return m_receiptCode;
}

h256 const& TransactionReceipt::getStateRoot() const
{
    return m_stateRoot;
}

// @override
bytes TransactionReceipt::getKey()
{
    return bytes();
}

// @override
bytes TransactionReceipt::getRLPData()
{
    core::RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}
}