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

using namespace net;

BytesPacket::BytesPacket(uint8_t cap)
{

}

BytesPacket::BytesPacket()
{

}

bytes const& BytesPacket::type() const
{
    return m_type;
}

bytes const& BytesPacket::data() const
{
    return m_data;
}

uint8_t BytesPacket::cap() const
{
    return m_cap;
}

size_t BytesPacket::size() const
{
    // return sizeof ()
    return 0;
}

void BytesPacket::setChainId(chain::ChainId id)
{
    m_chainId = id;
}

chain::ChainId BytesPacket::getChainId() const
{
    return m_chainId;
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