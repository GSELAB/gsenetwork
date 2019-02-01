#pragma once

//#include <vm/Memory.h>
//#include <vm/Stack.h>
#include <vm/VMInterface.h>
#include <vm/Exceptions.h>

namespace runtime {
namespace vm {

class VM {
    friend class Stack;
public:
    enum ExceptionType {
        BadInstruction = 0x01,
        BadJump = 0x02,
        BadStack = 0x03,
        RevertInstruction = 0x04,
        BufferOverrun = 0x05,
        BadOverflow = 0x06,
        OutOfGas = 0x07,
        DisallowedStateChange,
    };

    VM(VMInterface& extVM);

    bytes const& memory() const { return m_memory; }

    virtual void exec(OnOpFunc const& onOp);

protected:
    void initEntry();

    void optimize();

    void interpret();

    void create();

    bool callSetup(Arguments*, bytesRef& output);

    void call();

    void copyToMemory(bytesConstRef data, u256* sp);

    uint64_t memNeed(u256 offset, u256 size);

    void throwException(ExceptionType type);

    int64_t verifyJumpDest(u256 const& dest, bool th = true);

    void onOperation();

    void adjustStack(unsigned removed, unsigned added);

    void updateMem(uint64_t newMem);

    void fetchInstruction();

    uint64_t decodeJumpDest(byte* const code, uint64_t& pc);

    uint64_t decodeJumpvDest(byte* const code, uint64_t& pcA, byte voff);

    template<class T>
    uint64_t toInt63(T t) {
        // check for overflow
        if (t > 0x7FFFFFFFFFFFFFFF)
            throwException(OutOfGas);
        uint64_t w = uint64_t(t);
        return w;
    }

    template<class T>
    uint64_t toInt15(T t) {
        // check for overflow
        if (t > 0x7FFF)
            throwException(OutOfGas);
        uint64_t w = uint64_t(t);
        return w;
    }

private:
    static std::array<InstructionMetric, 256> c_metrics;
    static void initMetrics();
    static u256 exp256(u256 _base, u256 _exponent);
    void copyCode(int extra);

private:
    VMInterface& m_extVM;
    OnOpFunc m_onOp;
    bytes m_code;

    typedef void (VM::*FuncPtr)();
    FuncPtr m_next = 0;
    FuncPtr m_fail = 0;
    uint64_t m_steps = 0;
    VMSchedule* m_schedule = nullptr;

    owning_bytes_ref m_output;

    //Memory m_memory;
    //Stack m_stack;
    bytes m_memory;
    u256 m_stack[1024];
    u256* m_stackEnd;

    u256* m_SP;
    u256* m_SPP;
    bytes m_returnResult;

    Instruction m_OP;
    uint64_t m_PC = 0;

    uint64_t m_newMemSize = 0;
    uint64_t m_copyMemSize = 0;

    std::vector<uint64_t> m_beginSubs;
    std::vector<uint64_t> m_jumpDests;
};

// Convert from a 256-bit integer stack/memory entry into a 160-bit Address hash.
// Currently we just pull out the right (low-order in BE) 160-bits.
inline Address asAddress(u256 _item)
{
	return right160(h256(_item));
}

inline u256 fromAddress(Address _a)
{
	return (u160)_a;
}
}
}