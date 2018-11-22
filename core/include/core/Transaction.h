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

#include "core/Types.h"
#include "core/Address.h"
#include <core/Common.h>
#include <core/RLP.h>
#include <core/Object.h>

#include <crypto/Common.h>
#include <chain/Types.h>

using namespace crypto;

namespace core {


#define TRANSACTION_FIELDS_WITHOUT_SIG (7)
#define TRANSACTION_FIELDS_ALL (8)

class Transaction: public Object {
public:
    static const uint32_t ContractCreation;
    static const uint32_t ContractCall;

    enum Type {
        VoteType = 0x0,
        TransferType = 0x01,
        BeenProducerType = 0x02,
        UnknownType = 0xFFFFFFFF,
    };

    Transaction();

    Transaction(Transaction const& transaction);

    Transaction(chain::ChainID chainID, uint32_t type, Address const& sender, Address const& recipient,
        uint64_t timestamp, bytes const& data, uint64_t value);

    Transaction(bytesConstRef data);

    Transaction(bytes const& data);

    void streamRLP(RLPStream& rlpStream) const;

    void streamRLPContent(RLPStream& rlpStream) const;

    // @return the sha3 of the transaction not include signature
    h256 getHash() const;

    Transaction& operator=(Transaction const& transaction);

    void sign(Secret const& secret);

    bool operator==(Transaction const& transaction) const;

    bool operator!=(Transaction const& transaction) const;

    void setChainID(chain::ChainID chainID) { m_chainID = chainID; }

    void setType(uint32_t type) { m_type = type; }

    void setSender(Address const& sender) { m_sender = sender; }

    void setRecipient(Address const& recipient) { m_recipient = recipient; }

    void setTimestamp(uint64_t timestamp) { m_timestamp = timestamp; }

    void setData(bytes const& data) { m_data = data; }

    void setValue(uint64_t value) { m_value = value; }

    void setSignature(SignatureStruct const& sig) { m_signature = sig; }

    chain::ChainID getChainID() const { return m_chainID; }

    uint32_t getType() const { return m_type; }

    Address const& getSender() const { return m_sender; }

    Address const& getRecipient() const { return m_recipient; }

    int64_t getTimestamp() const { return m_timestamp; }

    bytes const& getData() const { return m_data; }

    uint64_t getValue() const { return m_value; }

    SignatureStruct const& getSignature() const { return m_signature; }

    virtual bytes getKey() override;

    virtual bytes getRLPData() override;

    virtual Object::ObjectType getObjectType() const override { return Object::TransactionType; }

private:
    chain::ChainID m_chainID;
    uint32_t m_type;
    Address m_sender;
    Address m_recipient;
    int64_t m_timestamp;
    bytes m_data;
    uint64_t m_value;
    SignatureStruct m_signature;
};

using Transactions = vector<Transaction>;

using TransactionPtr = std::shared_ptr<Transaction>;

using TxID = h256;
using TransactionID = TxID;

extern Transaction EmptyTransaction;

extern TransactionPtr EmptyTransactionPtr;

} /* end of namespace */