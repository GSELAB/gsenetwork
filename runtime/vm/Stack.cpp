#include <vm/Stack.h>
#include <vm/VM.h>

namespace runtime {
namespace vm {

Stack::Stack(VM* vm): m_vm(vm)
{
    m_stackEnd = &m_stack[1024];
    m_SP = m_stackEnd;
    m_SPP = m_SP;
}

size_t Stack::stackSize() const
{
    return m_stackEnd - m_SP;
}

void Stack::adjustStack(unsigned removed, unsigned added)
{
    m_SP = m_SPP;

    // adjust stack and check bounds
    m_SPP += removed;
    if (m_stackEnd < m_SPP)
        throwBadStack(removed, added);
    m_SPP -= added;
    if (m_SPP < m_stack)
        throwBadStack(removed, added);
}

void Stack::throwBadStack(unsigned removed, unsigned added)
{
    bigint size = m_stackEnd - m_SPP;
    if (size < removed) {
        //if (m_vm->m_fail)
        //    (*m_vm->m_fail)();
        // BOOST_THROW_EXCEPTION(StackUnderflow() << RequirementError((bigint)removed, size));
    } else {
        //if (m_vm->m_fail)
        //    (*m_vm->m_fail)();
        // BOOST_THROW_EXCEPTION(OutOfStack() << RequirementError((bigint)(added - removed), size));
    }
}
}
}