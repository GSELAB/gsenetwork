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

#include <core/Common.h>

namespace core {

/*
 * 0x01: Transactoon
 * 0x02: BlockHeader
 * 0x03: Block
 * 0x04: Account
 * 0x05: Producer
 * 0x06: SubChain
 *
 * 0x20: AttributeState
 */
class Object {
public:
    virtual ~Object() {}

    virtual std::string getRLPData() = 0;

    virtual std::string getKey() = 0;

    virtual uint8_t getObjectType() const = 0;

};

} // end namespace