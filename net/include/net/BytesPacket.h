#pragma once

#include <core/Common.h>
#include <core/RLP.h>

namespace net {

class BytesPacket {
public:
    BytesPacket(uint8_t cap, core::RLPStream& type, core::RLPStream& data): m_cap(cap), m_type(type.out()), m_data(data.out()) {}
    BytesPacket(uint8_t cap, core::bytesConstRef data);
    BytesPacket(BytesPacket const& p) = delete;
    BytesPacket(BytesPacket&& p): m_cap(p.m_cap), m_type(p.m_type), m_data(p.m_data) {}

    core::bytes const& type() const { return m_type; }

    core::bytes const& data() const { return m_data; }

    uint8_t cap() const { return m_cap; }

    size_t size() const;

    bool append(core::bytesConstRef in);

    virtual bool isValid() const; //  noexecept;

protected:
    uint8_t m_cap;
    core::bytes m_type;
    core::bytes m_data;
};

} // end namespace