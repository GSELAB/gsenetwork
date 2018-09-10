#pragma once

#include <crypto/Common.h>

using namespace core;

namespace crypto {

// Decrypts cipher using Secret key.
bool decrypt(Secret const& secret, bytesConstRef cipher, bytes& o_plaintext);

// Symmetric decryption.
bool decryptSym(Secret const& secret, bytesConstRef cipher, bytes& o_plaintext);

// Decrypt payload using ECIES standard with AES128-CTR.
bool decryptECIES(Secret const& secret, bytesConstRef cipher, bytes& o_plaintext);

// Decrypt payload using ECIES standard with AES128-CTR.
// @a _sharedMacData is shared authenticated data.
bool decryptECIES(Secret const& secret, bytesConstRef sharedMacData, bytesConstRef cipher, bytes& o_plaintext);

// Decrypts payload with specified IV/ctr using AES128-CTR.
bytesSec decryptAES128CTR(bytesConstRef k, h128 const& iv, bytesConstRef cipher);

// Decrypts payload with specified IV/ctr using AES128-CTR.
inline bytesSec decryptSymNoAuth(SecureFixedHash<16> const& k, h128 const& iv, bytesConstRef cipher) { return decryptAES128CTR(k.ref(), iv, cipher); }
inline bytesSec decryptSymNoAuth(SecureFixedHash<32> const& k, h128 const& iv, bytesConstRef cipher) { return decryptAES128CTR(k.ref(), iv, cipher); }

} // namespace crypto