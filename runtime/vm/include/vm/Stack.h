#pragma once

namespace runtime {

namespace vm {

class Stack {
public:
    Stack();

    size_t stackSize() const;

    void adjustStack(unsigned removed, unsigned added);

private:
    u256 m_stack[1024];
    u256* m_stackEnd;

    u256* m_SP;
    u256* m_SPP;
};
}
}