#pragma once

#include <core/All.h>

using namespace core;

namespace crypto {

bool sha3(bytesConstRef input, bytesRef result) noexcept;

inline h256 sha3(bytesConstRef input) noexcept
{
    h256 result;
    sha3(input, result.ref());
    return result;
}

template <unsigned N>
inline h256 sha3(FixedHash<N> const& input) noexcept
{
    return sha3(input.ref());
}

inline h256 sha3(bytes const& input)
{
    return sha3(bytesConstRef(&input));
}

extern h256 const EmptySHA3;
//extern h256 const EmptyListSHA3;

} // end of namespace