#pragma once

#include <core/Exceptions.h>

using namespace core;

namespace runtime {
namespace vm {

#define VM_TEMPLATE_EXCEPTION(name) \
struct name##Exception: virtual GSException { \
    explicit name##Exception(std::string const& message, int number = 0): \
        GSException(message, number) {} \
}

VM_TEMPLATE_EXCEPTION(BadInstruction);
VM_TEMPLATE_EXCEPTION(BadJump);
VM_TEMPLATE_EXCEPTION(OutOfGas);
VM_TEMPLATE_EXCEPTION(BadStack);
VM_TEMPLATE_EXCEPTION(StackUnderflow);
VM_TEMPLATE_EXCEPTION(BadOverflow);
VM_TEMPLATE_EXCEPTION(DisallowedStateChange);
VM_TEMPLATE_EXCEPTION(BufferOverrun);
}
}
