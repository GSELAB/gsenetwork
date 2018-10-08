#include <runtime/action/Transfer.h>

using namespace core;

namespace runtime {
namespace action {

// @override
void Transfer::init()
{

}

// @override
void Transfer::execute()
{
    Address const& sender = m_transaction.getSender();
    Address const& recipient = m_transaction.getRecipient();
    bytes const& data = m_transaction.getData();
    uint64_t value = m_transaction.getValue();
    m_repo->transfer(sender, recipient, value);
}

// @override
void Transfer::finalize()
{

}

}
}