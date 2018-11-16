#pragma once

#include <core/All.h>
#include <core/Log.h>

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

inline h256 sha3(std::pair<h256, h256> const& input) noexcept
{
    return sha3(bytesConstRef(toHex(input.first) + toHex(input.second)));
}

/// Calculate SHA3-256 hash of the given input (presented as a binary-filled string), returning as a 256-bit hash.
inline h256 sha3(std::string const& input) noexcept
{
    return sha3(bytesConstRef(input));
}

inline h256 sha3(bytes const& input)
{
    return sha3(bytesConstRef(&input));
}

// --------------------------------------------------------
inline SecureFixedHash<32> sha3Secure(bytesConstRef input) noexcept
{
    SecureFixedHash<32> ret;
    sha3(input, ret.writable().ref());
    return ret;
}

inline SecureFixedHash<32> sha3Secure(bytes const& input) noexcept
{
    return sha3Secure(bytesConstRef(&input));
}

inline SecureFixedHash<32> sha3Secure(std::string const& input) noexcept
{
    return sha3Secure(bytesConstRef(input));
}

template <unsigned N>
inline SecureFixedHash<32> sha3Secure(FixedHash<N> const& input) noexcept
{
    return sha3Secure(input.ref());
}

/// Calculate SHA3-256 MAC
inline void sha3mac(bytesConstRef secret, bytesConstRef plain, bytesRef output)
{
    sha3(secret.toBytes() + plain.toBytes()).ref().populate(output);
}

inline SecureFixedHash<32> sha3Secure(bytesSec const& input) noexcept
{
    return sha3Secure(input.ref());
}

//-------------------------------------------------
inline SecureFixedHash<32> sha3(bytesSec const& input) noexcept
{
    return sha3Secure(input.ref());
}

template <unsigned N>
inline SecureFixedHash<32> sha3(SecureFixedHash<N> const& input) noexcept
{
    return sha3Secure(input.ref());
}


template <unsigned N>
inline SecureFixedHash<32> sha3Secure(SecureFixedHash<N> const& input) noexcept
{
    return sha3Secure(input.ref());
}

extern h256 const EmptySHA3;
//extern h256 const EmptyListSHA3;

} // end of namespace
