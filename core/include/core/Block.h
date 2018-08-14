/**
 *  @author guoygang <hero.gariker@gmail.com>
 *  @date 2018
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