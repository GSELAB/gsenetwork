#pragma once

#include <chain/Types.h>
#include <core/Common.h>

using namespace core;
namespace net {

class BytesPacket {
public:
    BytesPacket(uint8_t cap);

    BytesPacket();

    bytes const& type() const;

    bytes const& data() const;

    uint8_t cap() const;

    size_t size() const;

    void setChainId(chain::ChainID id);

    chain::ChainID getChainId() const;

    bool append(bytesConstRef in);

    virtual bool isValid() const; //  noexecept;

    uint8_t getObjectType() const { return m_object; }

    void setObjectType(uint8_t object) { m_object = object; }

private:
    chain::ChainID m_chainId;
    uint8_t m_object;
    uint8_t m_cap;
    bytes m_type;
    bytes m_data;
};

} // end namespace