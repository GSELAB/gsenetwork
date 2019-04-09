#include <vm/Runtime.h>
#include <crypto/SHA3.h>

using namespace crypto;

namespace runtime {
namespace vm {

void Runtime::init()
{
    if (m_tx.getType() == Transaction::CreationType) {
        m_isCreation = true;
    } else {
        m_isCreation = false;
    }

}

void Runtime::execute()
{
    if (m_isCreation == true) {
        create();
    } else {
        call();
    }

    m_repo->put(m_tx);
}

bool Runtime::create()
{
    Address sender;
    u256 addressFlag = (u256)m_tx.getHash();
    m_creationAddress = right160(sha3(rlpList(sender, addressFlag)));
    if (m_repo->isAddressExist(m_creationAddress)) {
        revert();
        return true;
    }

    // Transfer value?
    {}

    {
        // Execute constructor function

    }

    int64_t timestamp = GENESIS_TIMESTAMP;
    if (m_block)
        timestamp = m_block->getTimestamp();

    Account account(m_creationAddress, Account::ContractType, timestamp);
    m_repo->put(account);
}

bool Runtime::call()
{
    Address const& sender = m_tx.getSender();
    Address const& target = m_tx.getRecipient();
    uint64_t value = m_tx.getValue();
    if (m_repo->isContractAddress(target)) {

    }

    m_repo->transfer(sender, target, value);
    return true;
}

bool Runtime::go()
{

}

void Runtime::revert()
{

}
}
}