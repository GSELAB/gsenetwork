//#include <mutex>

#include <runtime/jit/JITListener.h>

namespace runtime {
namespace jit {

// Global
//using Guard = std::lock_guard<std::mutex>;
//std::mutex x_jitListenerMutex;

CacheMode g_cacheMode;
std::unique_ptr<llvm::MemoryBuffer> g_lastObject;
JITListener* g_jitListener;

namespace DS {
JITCache* init(CacheMode mode, JITListener* listener)
{
    //Guard l{x_jitListenerMutex};

    g_cacheMode = mode;
    g_jitListener = listener;

    if (g_cacheMode == CacheMode::CLEAR) {
        clear();
        g_cacheMode = CacheMode::OFF;
    }

    if (g_cacheMode != CacheMode::OFF) {
        static JITCache s_jitCache;
        return &s_jitCache;
    }

    return nullptr;
}

void clear()
{

}

std::unique_ptr<llvm::Module> getObject(std::string const& id, llvm::LLVMContext& llvmContext)
{
    //Guard l{x_jitListenerMutex};
    if (g_cacheMode != CacheMode::ON && g_cacheMode != CacheMode::READ) return nullptr;


    return nullptr;
}
}


// ----------------------------------
void JITCache::notifyObjectCompiled(llvm::Module const* _module, llvm::MemoryBufferRef _object)
{
    //Guard l{x_jitListenerMutex};


}

std::unique_ptr<llvm::MemoryBuffer> JITCache::getObject(llvm::Module const* _module)
{
    //Guard l{x_jitListenerMutex};

    return std::move(g_lastObject);
}

}
}