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

// include signature
#define TRANSACTION_FIELDS (6 + 3)

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

    Transaction(chain::ChainID chainID, uint32_t type, Address const& sender, Address const& recipient, bytes const& data, uint64_t value);

    Transaction(bytesConstRef data);

    Transaction(bytes const& data);

    void streamRLP(RLPStream& rlpStream) const;

    // @return the sha3 of the transaction include signature
    h256 const& getHash();

    void sign(Secret const& secret);

    bool operator==(Transaction const& transaction) const;

    bool operator!=(Transaction const& transaction) const;

    void setChainID(chain::ChainID chainID);

    void setType(uint32_t type);

    void setSender(Address const& sender);

    void setRecipient(Address const& recipient);

    void setData(bytes const& data);

    void setValue(uint64_t value);

    chain::ChainID getChainID() const;

    uint32_t getType() const;

    Address const& getSender() const;

    Address const& getRecipient() const;

    bytes const& getData() const;

    uint64_t getValue() const;

    // @override
    std::string getKey();

    // @override
    std::string getRLPData();

    // @override
    uint8_t getObjectType() const { return 0x01; }

private:
    chain::ChainID m_chainID;
    uint32_t m_type;
    Address m_sender;
    Address m_recipient;
    bytes m_data;
    uint64_t m_value;
    SignatureStruct m_signature;

    h256 m_hash;
};

using Transactions = vector<Transaction>;

} /* end of namespace */