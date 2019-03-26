#include <vm/Memory.h>

namespace runtime {
namespace vm {

size_t Memory::size() const
{
    return m_cache.size();
}

bytes& Memory::getRef()
{
    return m_cache;
}

byte* Memory::data()
{
    return m_cache.data();
}

void Memory::resize(size_t newSize)
{
    m_cache.resize(newSize);
}
}
}