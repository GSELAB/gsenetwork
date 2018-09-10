#pragma once

#include <crypto/Common.h>

using namespace core;

namespace crypto {

// Encrypts plain text using Public key.
void encrypt(Public const& pubKey, bytesConstRef plain, bytes& o_cipher);

// Symmetric encryption.
void encryptSym(Secret const& secret, bytesConstRef plain, bytes& o_cipher);

/// Encrypt payload using ECIES standard with AES128-CTR.
void encryptECIES(Public const& pubKey, bytesConstRef plain, bytes& o_cipher);

/// Encrypt payload using ECIES standard with AES128-CTR.
/// @a _sharedMacData is shared authenticated data.
void encryptECIES(Public const& pubKey, bytesConstRef sharedMacData, bytesConstRef plain, bytes& o_cipher);

// Encrypts payload with random IV/ctr using AES128-CTR.
std::pair<bytes, h128> encryptSymNoAuth(SecureFixedHash<16> const& k, bytesConstRef plain);

// Encrypts payload with specified IV/ctr using AES128-CTR.
bytes encryptAES128CTR(bytesConstRef k, h128 const& iv, bytesConstRef plain);

// Encrypts payload with specified IV/ctr using AES128-CTR.
inline bytes encryptSymNoAuth(SecureFixedHash<16> const& k, h128 const& iv, bytesConstRef plain) { return encryptAES128CTR(k.ref(), iv, plain); }
inline bytes encryptSymNoAuth(SecureFixedHash<32> const& k, h128 const& iv, bytesConstRef plain) { return encryptAES128CTR(k.ref(), iv, plain); }
}