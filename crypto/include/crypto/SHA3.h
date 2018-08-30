#pragma once

#include <core/All.h>

using namespace core;

namespace crypto {

bool sha3(bytesConstRef _input, bytesRef o_output) noexcept;

inline h256 sha3(bytesConstRef _input) noexcept
{
    h256 ret;
    sha3(_input, ret.ref());
    return ret;
}

template <unsigned N>
inline h256 sha3(FixedHash<N> const& _input) noexcept
{
    return sha3(_input.ref());
}

} // end of namespace