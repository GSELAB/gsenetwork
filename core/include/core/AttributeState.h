#pragma once

#include <core/Object.h>
#include <core/Common.h>
#include <core/CommonIO.h>
#include <core/RLP.h>
#include <core/Exceptions.h>

namespace core {


class AttributeState: public core::Object {
public:
    enum Type: uint32_t {
        Uint64Type,
        BytesType,
        UnknownType,
    };

    AttributeState(bytes const& key, uint64_t value): m_key(key), m_type(Uint64Type), m_value(value) {}

    AttributeState(bytes const& key): m_key(key), m_type(BytesType) {}

    AttributeState(bytes const& key, bytesConstRef data);

    AttributeState& operator=(AttributeState const& as);

    // bool operator==(AttributeState<T> const& t) { return m_key == t.getKeyWord() && m_value == t.getValue(); }

    void streamRLP(RLPStream& rlpStream) const;

    Type getType() const { return m_type; }

    void setType(Type const& type) { m_type = type; }

    bytes const& getData() const { return m_data; }

    void setData(bytes const& data) { m_data = data; }

    uint64_t getValue() const { return m_value;}

    void setValue(uint64_t value) { m_value = value; }

    void setKey(bytes const& key) { m_key = key; }

    virtual bytes getRLPData() override;

    virtual bytes getKey() override { return m_key; }

    virtual Object::ObjectType getObjectType() const override { return Object::AttributeStateType; }

public:
    bytes m_key;
    Type m_type = UnknownType;
    uint64_t m_value = 0;
    bytes m_data;
};

#define ATTRIBUTE_GENESIS_INITED_KEY "attribute_genesis_inited_key"
extern AttributeState ATTRIBUTE_GENESIS_INITED;

#define ATTRIBUTE_DB_DIRTY_KEY "attribute_db_dirty_key"
extern AttributeState ATTRIBUTE_DB_DIRTY;

#define ATTRIBUTE_CURRENT_BLOCK_HEIGHT_KEY "attribute_current_block_height"
extern AttributeState ATTRIBUTE_CURRENT_BLOCK_HEIGHT;

// ATTRIBUTE_PREV_PRODUCER_LIST & ATTRIBUTE_CURRENT_PRODUCER_LIST used by producer server
#define ATTRIBUTE_PREV_PRODUCER_LIST_KEY "attribute_prev_producer_list"
extern AttributeState ATTRIBUTE_PREV_PRODUCER_LIST;

#define ATTRIBUTE_CURRENT_PRODUCER_LIST_KEY "attribute_current_producer_list"
extern AttributeState ATTRIBUTE_CURRENT_PRODUCER_LIST;

// solidify active producer list
#define ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST_KEY "attribute_solidify_active_producer_list"
extern AttributeState ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST;

}