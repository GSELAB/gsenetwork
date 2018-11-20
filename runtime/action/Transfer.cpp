#include <runtime/action/Transfer.h>

using namespace core;

namespace runtime {
namespace action {

void Transfer::init()
{

}

void Transfer::execute()
{
    Address const& sender = m_transaction.getSender();
    Address const& recipient = m_transaction.getRecipient();
    bytes const& data = m_transaction.getData();
    uint64_t value = m_transaction.getValue();
    m_repo->burn(sender, TRANSACTION_FEE);
    m_repo->transfer(sender, recipient, value);
}

void Transfer::finalize()
{

}
}
}