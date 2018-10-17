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

#include <net/BytesPacket.h>

using namespace core;

namespace net {

static bytesConstRef nextRLP(bytesConstRef _b)
{
    try {
        RLP r(_b, RLP::AllowNonCanon);
        return _b.cropped(0, std::min((size_t)r.actualSize(), _b.size()));
    } catch(...) {}
    return bytesConstRef();
}



BytesPacket::BytesPacket(uint8_t cap, bytesConstRef data):
    m_cap(cap), m_type(nextRLP(data).toBytes())
{
    if (data.size() > m_type.size()) {
        m_data.resize(data.size() - m_type.size());
        data.cropped(m_type.size()).copyTo(&m_data);
    }
}


size_t BytesPacket::size() const
{
    // return sizeof ()
    return 0;
}

bool BytesPacket::append(bytesConstRef in)
{
    auto offset = m_data.size();
    m_data.resize(offset + in.size());
    in.copyTo(bytesRef(&m_data).cropped(offset));
    return isValid();
}

bool BytesPacket::isValid() const
{
    return true;
}
} // namespace net






