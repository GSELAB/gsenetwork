#pragma once

#include <string>

#include <core/Common.h>

using namespace core;

namespace runtime {
namespace vm {

class Memory {
public:
    Memory() = default;

    virtual ~Memory() = default;

    size_t size() const;

    bytes& getRef();

    byte* data();

    void resize(size_t newSize);

private:
    bytes m_cache;
};
}
}