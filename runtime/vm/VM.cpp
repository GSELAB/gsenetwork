#include <vm/VM.h>

namespace runtime {
namespace vm {

void VM::initEntry()
{

}

void VM::optimize()
{

}

void VM::interpret()
{

}

void VM::create()
{

}

bool VM::callSetup(Arguments*, bytesRef& output)
{

}

void VM::call()
{

}

void VM::copyToMemory(u256* sp, bytesConstRef data)
{

}

uint64_t VM::memNeed(u256 offset, u256 size)
{

}

void VM::throwException(ExceptionType type)
{

}

int64_t VM::verifyJumpDest(u256 const& dest, bool th = true)
{

}

void VM::onOperation()
{

}

void VM::adjustStack(unsigned removed, unsigned added)
{

}

void VM::updateMem(uint64_t newMem)
{

}

void VM::fetchInstruction()
{

}

uint64_t VM::decodeJumpDest(byte* const code, uint64_t& pc)
{

}

uint64_t VM::decodeJumpvDest(byte* const code, uint64_t& pc, byte voff)
{

}

void VM::exec(OnOpFunc const& onOp)
{
    m_onOp = onOp;
    m_PC = 0;

    try {
        m_next = &VM::initEntry();
        do {
            (this->*m_next)();
        } while (m_next)
    } catch (...) {
        throw;
    }
}
}
}