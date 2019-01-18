#pragma once

#include <vm/Memory.h>
#include <vm/Stack.h>

namespace runtime {

namespace vm {

class VM {
public:
    enum ExceptionType {
        BadInstruction = 0x01,
        BadJump = 0x02,
        BadStack = 0x03,
        RevertInstruction = 0x04,
        BufferOverrun = 0x05,
        BadOverflow = 0x06,
    };

    VM(VMInterface& extVM): m_extVM(extVM) {}

    bytes const& memory() const { return m_mem; }

    virtual void exec(OnOpFunc const& onOp);

protected:
    void initEntry();

    void optimize();

    void interpret();

    void create();

    bool callSetup(Arguments*, bytesRef& output);

    void call();

    void copyToMemory(u256* sp, bytesConstRef data);

    uint64_t memNeed(u256 offset, u256 size);

    void throwException(ExceptionType type);

    int64_t verifyJumpDest(u256 const& dest, bool th = true);

    void onOperation();

    void adjustStack(unsigned removed, unsigned added);

    void updateMem(uint64_t newMem);

    void fetchInstruction();

    uint64_t decodeJumpDest(byte* const code, uint64_t& pc);

    uint64_t decodeJumpvDest(byte* const code, uint64_t& pc, byte voff);

    template<class T>
    uint64_t toInt63(T t) {
        // check for overflow
        if (t > 0x7FFFFFFFFFFFFFFF)
            throwOutOfGas();
        uint64_t w = uint64_t(t);
        return w;
    }

    template<class T>
    uint64_t toInt15(T t) {
        // check for overflow
        if (t > 0x7FFF)
            throwOutOfGas();
        uint64_t w = uint64_t(t);
        return w;
    }

private:
    VMInterface& m_extVM;
    OnOpFunc m_onOp;
    bytes m_code;

    typedef void (VM::*FuncPtr)();
    FuncPtr m_next;
    FuncPtr m_fail;
    uint64_t m_steps;


    Memory m_memory;
    Stack m_stack;
    bytes m_returnResult;

    Instruction m_OP;
    uint64_t m_PC = 0;

    std::vector<uint64_t> m_beginSubs;
    std::vector<uint64_t> m_jumpDests;
};
}
}