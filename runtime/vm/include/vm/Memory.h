#pragma once

namespace runtime {
namespace vm {

class Memory {
public:
    Memory() = default;

    virtual ~Memory() = default;


private:
    bytes m_cache;
};
}
}