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
        Uint32Type,
        Uint64Type,
        BytesType,
        UnknownType,
    };

    AttributeState(bytes const& key, T const& value):m_key(key) {
        if (typeid(value) == typeid(bool)) {
            m_type = BoolType;
            m_value = (uint64_t)value;
        } else if (typeid(value) == typeid(uint32_t)) {
            m_type = Uint32Type;
            m_value = (uint64_t)value;
        } else if (typeid(value) == typeid(uint64_t)) {
            m_type = Uint64Type;
            m_value = (uint64_t)value;
        } else if (typeid(value) == typeid(bytes)) {
            m_type = BytesType;
            m_data = (bytes)value;
        } else {
            m_type = UnknownType;
        }
    }

#define CONSTANT_STATE_FIELDS (2)

    AttributeState(bytes const& key, bytesConstRef data):m_key(key) {
        try {
            core::RLP rlp(data);
            if (rlp.isList() && rlp.itemCount() == CONSTANT_STATE_FIELDS) {
                m_type = (Type)rlp[0].toInt<uint32_t>();
                switch (m_type) {
                    case BoolType:
                        if (typeid(T) != typeid(bool))
                            THROW_GSEXCEPTION("Attribute type is not bool");
                            //BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (bool)"));
                        m_value = (uint64_t)rlp[1].toInt<uint8_t>();
                        break;
                    case Uint32Type:
                        if (typeid(T) != typeid(uint32_t))
                            THROW_GSEXCEPTION("Attribute type is not uint32_t");
                            //BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (uint32_t)"));
                        m_value = (uint64_t)rlp[1].toInt<uint32_t>();
                        break;
                    case Uint64Type:
                        if (typeid(T) != typeid(uint64_t))
                            THROW_GSEXCEPTION("Attribute type is not uint64_t");
                            //BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (uint64_t)"));
                        m_value = (uint64_t)rlp[1].toInt<uint64_t>();
                        break;
                    case BytesType:
                        if (typeid(T) != typeid(bytes))
                            THROW_GSEXCEPTION("Attribute type is not bytes");
                            //BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (bytes)"));
                        m_data = rlp[1].toBytes();
                        break;
                    default:
                        THROW_GSEXCEPTION("Attribute type is not any type");
                        // BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (unknown)"));
                        break;
                }
            } else {
                THROW_GSEXCEPTION("None list || None CONSTANT_STATE_FIELDS");
                //BOOST_THROW_EXCEPTION(std::range_error("None list || None CONSTANT_STATE_FIELDS"));
            }
        } catch (Exception e) {
            THROW_GSEXCEPTION("Attribute interpret failed");
            //BOOST_THROW_EXCEPTION(std::range_error("Invalid ConstantState format!"));
        }
    }

    bool operator==(AttributeState<T> const& t) { return m_key == t.getKeyWord() && m_value == t.getValue(); }

    void streamRLP(core::RLPStream& rlpStream) const {
        rlpStream.appendList(CONSTANT_STATE_FIELDS);
        rlpStream << m_type;
        switch (m_type) {
            case BoolType:
            case Uint32Type:
            case Uint64Type:
                rlpStream << m_value;
                break;
            case BytesType:
                rlpStream << m_data;
                break;
            default:
                //BOOST_THROW_EXCEPTION(std::range_error("Invalid ConstantState format!"));
                THROW_GSEXCEPTION("Invalid attribute format!");
                break;
        }
    }

    bytes getKeyWord() const { return m_key; }

    Type getType() const { return m_type; }

    void setType(Type const& type) { m_type = type; }

    bytes const& getData() const { return m_data; }

    void setData(bytes const& data) { m_data = data; }

    T getValue() const { return (T)m_value;}

    void setValue(T const& value) { m_value = value; }

    // @ override
    bytes getRLPData() { core::RLPStream rlpStream; streamRLP(rlpStream); return rlpStream.out(); /* return bytesConstRef(&rlpStream.out()).toString(); */  }

    // @ override
    bytes getKey() { return m_key; }

    //@override
    Object::ObjectType getObjectType() const { return Object::AttributeStateType; }
private:
    bytes m_key;
    Type m_type;
    uint64_t m_value = 0;
    bytes m_data;
};

#define ATTRIBUTE_GENESIS_INITED_KEY "attribute_genesis_inited_key"
extern AttributeState<bool> ATTRIBUTE_GENESIS_INITED;

#define ATTRIBUTE_CURRENT_BLOCK_HEIGHT_KEY "attribute_current_block_height"
extern AttributeState<uint64_t> ATTRIBUTE_CURRENT_BLOCK_HEIGHT;

#define ZERO_BLOCK_HEIGHT (0)
extern Block ZeroBlock;

} // end namespace
