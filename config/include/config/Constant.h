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

#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>

using namespace core;

namespace config {
//
#define MAX_TRANSACTION_QUEUE_SIZE  (1024 * 128)

#define MAX_BLOCK_QUEUE_SIZE (1024 * 2)

// Transaction size 1 KB()
#define MAX_TRANSACTION_SIZE 1024

// Max block size 1024 KB (1 M)
#define MAX_BLOCK_SIZE (1024 * 1024 * 1204)


/*
 * @ Constant State
 * @ support bool\uint32_t\uint64_t\bytes
 */
template<typename T>
class ConstantState: public core::Object {
public:
    enum Type: uint32_t {
        BoolType = 0,
        Uint32Type,
        Uint64Type,
        BytesType,
        UnknownType,
    };

    ConstantState(std::string const& key, T const& value):m_key(key) {
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

    ConstantState(std::string const& key, bytesConstRef const& data):m_key(key) {
        try {
            RLP rlp(data);
            if (rlp.isList() && rlp.itemCount() == CONSTANT_STATE_FIELDS) {
                m_type = (Type)rlp[0].toInt<uint32_t>();
                switch (m_type) {
                    case BoolType: {
                        if (typeid(T) != typeid(bool)) {
                            BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (bool)"));
                        }

                        m_value = (uint64_t)rlp[1].toInt<uint8_t>();
                        break;
                    }
                    case Uint32Type: {
                        if (typeid(T) != typeid(uint32_t)) {
                            BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (uint32_t)"));
                        }

                        m_value = (uint64_t)rlp[1].toInt<uint32_t>();
                        break;
                    }
                    case Uint64Type: {
                        if (typeid(T) != typeid(uint64_t)) {
                            BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (uint64_t)"));
                        }

                        m_value = (uint64_t)rlp[1].toInt<uint64_t>();
                        break;
                    }
                    case BytesType: {
                        if (typeid(T) != typeid(bytes)) {
                            BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (bytes)"));
                        }

                        m_data = rlp[1].toBytes();
                        break;
                    }
                    default: {
                        BOOST_THROW_EXCEPTION(std::range_error("Constant type is invliad (unknown)"));
                        break;
                    }

                }
            }
        } catch (Exception e) {
            BOOST_THROW_EXCEPTION(std::range_error("Invalid ConstantState format!"));
        }
    }

    bool operator==(T const& t) {
        return (m_key == t.getKeyWord() &&
            m_value == t.getValue());
    }

    void streamRLP(RLPStream& rlpStream) const {
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
            default: {
                BOOST_THROW_EXCEPTION(std::range_error("Invalid ConstantState format!"));
                break;
            }
        }
    }

    std::string const& getKeyWord() const {
        return m_key;
    }

    Type getType() const {
        return m_type;
    }

    bytes const& getData() const {
        return m_data;
    }

    T getValue() const {
        return (T)m_value;
    }

    // @ override
    std::string getRLPData() {
        RLPStream rlpStream;
        streamRLP(rlpStream);
        return bytesConstRef(&rlpStream.out()).toString();
    }

    // @ override
    std::string getKey() {
        return m_key;
    }

private:
    std::string m_key;
    Type m_type;
    uint64_t m_value;
    bytes m_data;
};

#define ATTRIBUTE_GENESIS_INITED_KEY "attribute_genesis_inited_key"
extern ConstantState<bool> ATTRIBUTE_GENESIS_INITED;

} // end namespace