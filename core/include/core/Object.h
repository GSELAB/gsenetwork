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
    enum ObjectType: uint16_t {
        AccountType             = 0x01,
        TransactionType         = 0x02,
        BlockHeaderType         = 0x03,
        BlockType               = 0x04,
        ProducerType            = 0x05,
        TransactionReceiptType  = 0x06,
        BallotType              = 0x07,
        SubChainType            = 0x08,
        CandidateType           = 0x09,
        AttributeStateType      = 0x0A,
        HeaderConfirmationType  = 0x0B,
        BlockStateType          = 0x0C,
        ProducerScheduleTypeType = 0x0D,
        /* Add new type */

        UnknownType = 0xFFFF,

    };

    virtual ~Object() {}

    virtual bytes getRLPData() = 0;

    virtual bytes getKey() = 0;

    virtual ObjectType getObjectType() const = 0;

};

} // end namespace