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

#include <core/Object.h>
#include <core/RLP.h>
#include <core/Log.h>
#include <core/Block.h>
#include <core/Exceptions.h>

#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>

using namespace core;

namespace config {

// Genesis block time stamp is 2018-10-01 UTC 00:00:00 (1538352000)
#define GENESIS_TIMESTAMP (0x5BB16380)

//
#define MAX_TRANSACTION_QUEUE_SIZE  (1024 * 128)

#define MAX_BLOCK_QUEUE_SIZE (1024 * 2)

// Max transaction size 512 Bytes (0.5 KB)
#define MAX_TRANSACTION_SIZE (512)

// Maximum 750 transactions per block
#define MAX_TRANSACTIONS_PER_BLOCK (750)

// Max block size 1024 KB (1 M)
#define MAX_BLOCK_SIZE (1024 * 1024)

// 21 delegated blocks
#define NUM_DELEGATED_BLOCKS (7)    //21

// 0.1 second = 100 ms
#define PRODUCER_SLEEP_INTERVAL  (100)

// 2 second = 2000 ms
#define PRODUCER_INTERVAL (2000)

/*
 * @ Constant State
 * @ support bool\uint32_t\uint64_t\bytes
 */
template<typename T>
class AttributeState: public core::Object {
public:
    enum Type: uint32_t {
        BoolType = 0,
        Uint64Type,
        BytesType,
        UnknownType,
    };

    AttributeState(bytes const& key): m_key(key), m_type(UnknownType) {}

    AttributeState(bytes const& key, bool value): m_key(key), m_type(BoolType) { m_value = (uint64_t)value; }

    AttributeState(bytes const& key, uint64_t value): m_key(key), m_type(Uint64Type), m_value(value) {}

    AttributeState(bytes const& key, bytes const& data): m_key(key), m_type(BytesType), m_data(data) {}

    AttributeState(bytes const& key, bytesConstRef data): m_key(key) {
        try {
            core::RLP rlp(data);
            if (rlp.isList() && rlp.itemCount() == 2) {
                m_type = (Type)rlp[0].toInt<uint32_t>();
                switch (m_type) {
                    case BoolType:
                        if (typeid(T) != typeid(bool)) THROW_GSEXCEPTION("Attribute type is not bool");
                        m_value = (uint64_t)rlp[1].toInt<uint8_t>();
                        break;
                    case Uint64Type:
                        if (typeid(T) != typeid(uint64_t)) THROW_GSEXCEPTION("Attribute type is not uint64_t");
                        m_value = (uint64_t)rlp[1].toInt<uint64_t>();
                        break;
                    case BytesType:
                        if (typeid(T) != typeid(bytes)) THROW_GSEXCEPTION("Attribute type is not bytes");
                        m_data = rlp[1].data().toBytes();
                        break;
                    default:
                        THROW_GSEXCEPTION("Attribute type is not any type");
                        break;
                }
            } else {
                THROW_GSEXCEPTION("None list || None CONSTANT_STATE_FIELDS");
            }
        } catch (Exception e) {
            THROW_GSEXCEPTION("Attribute interpret failed");
        }
    }

    AttributeState& operator=(AttributeState const& as) {
        if (this == &as) return *this;
        m_key = as.m_key;
        m_type = as.m_type;
        m_value = as.m_value;
        m_data = as.m_data;
        return *this;
    }

    bool operator==(AttributeState<T> const& t) { return m_key == t.getKeyWord() && m_value == t.getValue(); }

    void streamRLP(core::RLPStream& rlpStream) const {
        rlpStream.appendList(2);
        rlpStream << m_type;
        if (m_type == BoolType || m_type == Uint64Type) {
            rlpStream << m_value;
        } else if (m_type == BytesType) {
            rlpStream.appendRaw(m_data);
        }
    }

    bytes getKeyWord() const { return m_key; }

    Type getType() const { return m_type; }

    void setType(Type const& type) { m_type = type; }

    bytes const& getData() const { return m_data; }

    void setData(bytes const& data) { m_data = data; }

    T getValue() const { return (T)m_value;}

    void setValue(T const& value) { m_value = value; }

    virtual bytes getRLPData() override { core::RLPStream rlpStream; streamRLP(rlpStream); return rlpStream.out();  }

    virtual bytes getKey() override { return m_key; }

    virtual Object::ObjectType getObjectType() const override { return Object::AttributeStateType; }

public:
    bytes m_key;
    Type m_type;
    uint64_t m_value = 0;
    bytes m_data;
};

#define ATTRIBUTE_GENESIS_INITED_KEY "attribute_genesis_inited_key"
extern AttributeState<bool> ATTRIBUTE_GENESIS_INITED;

#define ATTRIBUTE_CURRENT_BLOCK_HEIGHT_KEY "attribute_current_block_height"
extern AttributeState<uint64_t> ATTRIBUTE_CURRENT_BLOCK_HEIGHT;

#define ATTRIBUTE_PREV_PRODUCER_LIST_KEY "attribute_prev_producer_list"
extern AttributeState<bytes> ATTRIBUTE_PREV_PRODUCER_LIST;

#define ATTRIBUTE_CURRENT_PRODUCER_LIST_KEY "attribute_current_producer_list"
extern AttributeState<bytes> ATTRIBUTE_CURRENT_PRODUCER_LIST;


} // end namespace
