#pragma once

#include <core/Common.h>

using namespace core;

namespace runtime {
namespace vm {

class VM;

class Stack {
public:
    Stack(VM* vm);

    size_t stackSize() const;

    void adjustStack(unsigned removed, unsigned added);

    u256 operator[](int index) const {
        if (index < 0 || index >= 1024) {
            return u256();
        }

        return m_stack[index];
    }

    Stack const& operator++(int) {
        return *this;
    }

    void setSPP(int index, u256 value) {
        m_SPP[index] = value;
    }

protected:
    void throwBadStack(unsigned removed, unsigned added);

private:
    VM* m_vm;

    u256 m_stack[1024];
    u256* m_stackEnd;

    u256* m_SP;
    u256* m_SPP;
};
}
}