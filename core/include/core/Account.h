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

#ifndef __ACCOUNT_HEADER__
#define __ACCOUNT_HEADER__

#include "Timestamp.h"

namespace core {

class Account {
public:

private:

    bool m_isAlive = false;

    uint64_t m_balance;

    Timestamp m_createdTime;
};

} /* namespace */

#endif