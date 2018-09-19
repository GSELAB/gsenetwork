#pragma once

#include <memory>
#include <unordered_map>

#include <llvm/ExecutionEngine/ObjectCache.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace runtime {
namespace jit {

enum class JITState {
    Start,
    CacheLoad,
    CacheWrite,
    Compilation,
    Optimization,
    CodeGen,
    Execution,
    Return,
    Finished,

};

class JITListener {
public:
    JITListener() = default;

    virtual void executionStarted() {}
    virtual void executionEnded() {}

    virtual void stateChanged(JITState) {}
};

enum class CacheMode {
    OFF,
    ON,
    READ,
    WRITE,
    CLEAR,
    PRELOAD,

};

class JITCache: public llvm::ObjectCache {
public:
	/// notifyObjectCompiled - Provides a pointer to compiled code for Module M.
	virtual void notifyObjectCompiled(llvm::Module const* _module, llvm::MemoryBufferRef _object) final override;

	/// getObjectCopy - Returns a pointer to a newly allocated MemoryBuffer that
	/// contains the object which corresponds with Module M, or 0 if an object is
	/// not available. The caller owns both the MemoryBuffer returned by this
	/// and the memory it references.
	virtual std::unique_ptr<llvm::MemoryBuffer> getObject(llvm::Module const* _module) final override;
};

namespace DS {
JITCache* init(CacheMode mode, JITListener* listener);
std::unique_ptr<llvm::Module> getObject(std::string const& id, llvm::LLVMContext& llvmContext);
void clear();
}
}
}