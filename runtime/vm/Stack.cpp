#include <vm/Stack.h>

namespace runtime {
namespace vm {

Stack::Stack()
{
    m_stackEnd = &m_stack[1024];
    m_SP = m_stackEnd;
    m_SPP = m_SP;
}

size_t Stack::stackSize() const
{
    return m_stackEnd - mSP;
}

void Stack::adjustStack(unsigned removed, unsigned added)
{

}
}
}