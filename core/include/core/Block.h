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

#include "core/Transaction.h"
//#include "Timestamp.h"
#include "core/Types.h"
#include "core/Address.h"

namespace core {

class BlockHeader {
public:

private:
    //Timestamp   m_timestamp;
    Address  producer;

    uint64_t m_number;



};

class Block : public BlockHeader {
public:

private:
    Transactions m_transactions;


};

}  /* namespace end */