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

#include <stdint.h>
#include <stddef.h>

namespace chain {

using ChainID = uint64_t;

enum ChainDomain : ChainID {
    GSE_UNKNOWN_NETWORK         = 0xFFFFFFFFFFFFFFFF,   // < unknown

    GSE_ROOT_NETWORK            = 0x000010,                     // < 0x0000 ~ 0x00FF : used for root chain id
    GSE_ROOT_NETWORK_END        = 0x0000FF,

    GSE_ROOT_NETWORK_TEST       = 0x000100,                     // < 0x0100 ~ 0x01FF : used for test network chain id
    GSE_ROOT_NETWORK_TEST_END   = 0x0001FF,

    GSE_SUB_NETWORK             = 0x010000,                     // < 0x010000 : the start of subchain
};

#define DEFAULT_GSE_NETWORK GSE_ROOT_NETWORK

}