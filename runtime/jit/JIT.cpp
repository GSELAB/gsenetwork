#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/Triple.h>

#include <runtime/jit/JIT.h>

#ifdef __INNER_WORK__
#include <core/Log.h>
#endif

namespace runtime {

namespace jit {

using JITFunc = JITReturn(*)(JIT*);

struct CodeMapEntry {
    JITFunc func = nullptr;
    size_t hits = 0;

    CodeMapEntry() = default;
    explicit CodeMapEntry(JITFunc func) : func(func) {}
};


class SymbolResolver: public llvm::SectionMemoryManager {
public:
    size_t totalMemorySize() const { return m_totalMemorySize; }

private:
    llvm::JITSymbol findSymbol(std::string const& name) override {
        {
            // TODO: SELF
        }
        return llvm::SectionMemoryManager::findSymbol(name);
    }

    void reportMemorySize(size_t inc) {
        if (((uint64_t)m_totalMemorySize + (uint64_t)inc) < (uint64_t)inc) {
#ifdef __INNER_WORK__
            CERROR << "Overflow occur";
#endif
            // THROW EXCEPTION
            return;
        }

        m_totalMemorySize += inc;
        if (m_totalMemorySize >= m_printMemoryLimit) {
            size_t memstep = 10 * 1024 * 1024;
#ifdef __INNER_WORK__
            CERROR << "m_totalMemorySize >= m_printMemoryLimit(" << (m_printMemoryLimit / (1024 * 1024)) << " MB)";
#endif
            m_printMemoryLimit += memstep;
        }
    }

    uint8_t* allocateCodeSection(uintptr_t size, unsigned a, unsigned id, llvm::StringRef name) override {
        reportMemorySize(size);
        return llvm::SectionMemoryManager::allocateCodeSection(size, a, id, name);
    }

    uint8_t* allocateDataSection(uintptr_t size, unsigned a, unsigned id, llvm::StringRef name, bool ro) override {
        reportMemorySize(size);
        return llvm::SectionMemoryManager::allocateDataSection(size, a, id, name, ro);
    }

    size_t m_totalMemorySize = 0;
    size_t m_printMemoryLimit = 1024 * 1024;
};

class JITImpl {
public:
#define MAX_MEMORY_LIMIT (1000 * 1024 * 1024)

    static llvm::LLVMContext& getLLVMContext() {
        static llvm::LLVMContext s_llvmContet;
        return s_llvmContet;
    }

    static JITImpl& instance() {
        static JITImpl s_jitImpl;
        return s_jitImpl;
    }

    llvm::ExecutionEngine& engine() { return *m_engine; }

    void compile(byte const* code, size_t size) {

    }

    void checkMemorySize() {
        if (m_memMgr->totalMemorySize() > MAX_MEMORY_LIMIT) {
            // RESET JIT ENGINE
            std::lock_guard<std::mutex> l{x_codeMap};
            m_codeMap.clear();
            m_engine.reset();
            createEngine();
        }
    }

private:
    JITImpl() {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        createEngine();
    }

    void createEngine() {
        auto module = llvm::make_unique<llvm::Module>("", getLLVMContext());
        auto triple = llvm::Triple(llvm::sys::getProcessTriple());
        if (triple.getOS() == llvm::Triple::OSType::Win32) {
#ifdef __INNER_WORK__
            CERROR << "Not support Win32";
#endif
            return;
        }

        llvm::EngineBuilder builder(std::move(module));
        builder.setEngineKind(llvm::EngineKind::JIT);
        auto memMgr = llvm::make_unique<SymbolResolver>();
        m_memMgr = memMgr.get();
        builder.setMCJITMemoryManager(std::move(memMgr));
        builder.setOptLevel(llvm::CodeGenOpt::Default);
        builder.setVerifyModules(true);
        m_engine.reset(builder.create());

        // SET CACHE LISTENER
        m_engine->setObjectCache(DS::init(CacheMode::ON, nullptr));
    }

private:
    std::unique_ptr<llvm::ExecutionEngine> m_engine;
    SymbolResolver const* m_memMgr = nullptr;

    mutable std::mutex x_codeMap;
    std::unordered_map<std::string, CodeMapEntry> m_codeMap;

    std::vector<uint8_t> m_retBuffer;
    std::vector<uint8_t> m_codeBuffer;

};

void JIT::init()
{

}

void JIT::run()
{

}



}
}
