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

#include <core/Common.h>
#include <core/FixedHash.h>
#include <core/RLP.h>
#include <core/Object.h>

namespace core {

#define TRANSACTION_RECEIPT_FIELDS (1)

class TransactionReceipt {
public:
    TransactionReceipt(bytesConstRef data);

    TransactionReceipt(uint32_t status);

    TransactionReceipt(h256 const& root);

    void streamRLP(RLPStream& rlpStream) const;

    bytes rlpBytes() const;

    bool hasReceiptCode() const;

    uint32_t getReceiptCode() const;

    h256 const& getStateRoot() const;

    // @override
    bytes getKey();

    // @override
    bytes getRLPData();

    // @override
    Object::ObjectType getObjectType() const { return Object::TransactionReceiptType; }

private:
    uint32_t m_receiptCode;
    h256 m_stateRoot;
    bool m_isCode;
};

using TransactionReceipts = std::vector<TransactionReceipt>;

} // end namespace