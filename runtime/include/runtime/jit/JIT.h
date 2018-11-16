#pragma once

#include <runtime/jit/JITListener.h>
#include <llvm/IR/IRBuilder.h>

#ifndef __INNER_WORK__
#define __INNER_WORK__
#endif
namespace runtime {
namespace jit {

using byte = uint8_t;
using IRBuilder = llvm::IRBuilder<>;

enum class JITReturn {
    // Success codes
	Stop    = 0,
	Return  = 1,
	Revert  = 2,

	// Standard error codes
	OutOfGas           = -1,

	// Internal error codes
	LLVMError          = -101,

	UnexpectedException = -111,
};

class JIT {
public:
    JIT() {}
    ~JIT() {}

    void init();

    void run();

private:

};
}
}