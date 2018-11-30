#include <core/AttributeState.h>
#include <core/Log.h>

namespace core {

AttributeState ATTRIBUTE_GENESIS_INITED(toBytes(ATTRIBUTE_GENESIS_INITED_KEY), 1);

AttributeState ATTRIBUTE_DB_DIRTY(toBytes(ATTRIBUTE_DB_DIRTY_KEY), 0);

AttributeState ATTRIBUTE_CURRENT_BLOCK_HEIGHT(toBytes(ATTRIBUTE_CURRENT_BLOCK_HEIGHT_KEY), (uint64_t)0);

AttributeState ATTRIBUTE_PREV_PRODUCER_LIST(toBytes(ATTRIBUTE_PREV_PRODUCER_LIST_KEY));

AttributeState ATTRIBUTE_CURRENT_PRODUCER_LIST(toBytes(ATTRIBUTE_CURRENT_PRODUCER_LIST_KEY));

AttributeState ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST(toBytes(ATTRIBUTE_SOLIDIFY_ACTIVE_PRODUCER_LIST_KEY));

AttributeState::AttributeState(bytes const& key, bytesConstRef data): m_key(key)
{
    try {
        core::RLP rlp(data);
        if (rlp.isList() && rlp.itemCount() == 2) {
            m_type = (Type)rlp[0].toInt<uint32_t>();
            if (m_type == Uint64Type) {
                m_value = (uint64_t)rlp[1].toInt<uint64_t>();
            } else if (m_type == BytesType) {
                m_data = rlp[1].data().toBytes();
            }
        } else {
            throw AttributeStateException("None list || None CONSTANT_STATE_FIELDS");
        }
    } catch (AttributeStateException& e) {
        throw e;
    } catch (Exception& e) {
        throw e;
    }
}

AttributeState& AttributeState::operator=(AttributeState const& as)
{
    if (this == &as) return *this;
    m_key = as.m_key;
    m_type = as.m_type;
    if (m_type == Uint64Type) {
        m_value = as.m_value;
    } else if (m_type == BytesType) {
        m_data = as.m_data;
    }
    return *this;
}

void AttributeState::streamRLP(RLPStream& rlpStream) const
{
    rlpStream.appendList(2);
    rlpStream << m_type;
    if (m_type == Uint64Type) {
        rlpStream << (bigint) m_value;
    } else if (m_type == BytesType) {
        rlpStream.appendRaw(m_data);
    }
}

bytes AttributeState::getRLPData()
{
    RLPStream rlpStream;
    streamRLP(rlpStream);
    return rlpStream.out();
}

}
