/**
 *  @author guoygang <hero.gariker@gmail.com>
 *  @date 2018
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