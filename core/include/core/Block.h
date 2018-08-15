/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of gsenetwork.
 *
 * gsenetwork is is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

/*
 * @author guoygang <hero.gariker@gmail.com>
 * @date 2018
 */

#ifndef __BLOCK_HEADER__
#define __BLOCK_HEADER__

#include "Transaction.h"
#include "Timestamp.h"
#include "Types.h"

namespace core {

class BlockHeader {
public:

private:
    Timestamp   m_timestamp;
    xxxxxx  producer;

    uint64_t m_number;



};

class Block : public BlockHeader {
public:

private:
    Transactions m_transactions;


};

}  /* namespace end */
#endif