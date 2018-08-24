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

    void setChainId(chain::ChainId id);

    chain::ChainId getChainId() const;

    bool append(bytesConstRef in);

    virtual bool isValid() const; //  noexecept;

private:
    chain::ChainId m_chainId;
    uint8_t m_cap;
    bytes m_type;
    bytes m_data;

};

} // end namespace