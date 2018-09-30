/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Common.h
 * @author Alex Leverington <nessence@gmail.com>
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Ethereum-specific data structures & algorithms.
 */

#pragma once

#include <mutex>
#include <core/Address.h>
#include <core/Common.h>
#include <core/Exceptions.h>
#include <core/FixedHash.h>

namespace crypto
{

using Secret = core::SecureFixedHash<32>;

using Public = core::h512;

using Signature = core::h520;

struct SignatureStruct {
	SignatureStruct() = default;
	SignatureStruct(Signature const& _s) { *(core::h520*)this = _s; }
	SignatureStruct(core::h256 const& _r, core::h256 const& _s, byte _v): r(_r), s(_s), v(_v) {}
	operator Signature() const { return *(core::h520 const*)this; }

	/// @returns true if r,s,v values are valid, otherwise false
	bool isValid() const noexcept;

	SignatureStruct& operator=(SignatureStruct const& sig) {
	    r = sig.r;
	    s = sig.s;
	    v = sig.v;
	    return *this;
	}

	bool operator==(SignatureStruct const& sig) const {
	    return (r == sig.r) && (s == sig.s) && (v == sig.v);
    }

	core::h256 r;
	core::h256 s;
	byte v = 0;
};

using Secrets = std::vector<Secret>;

Public toPublic(Secret const& secret);

core::Address toAddress(Public const& pubKey);

core::Address toAddress(Secret const& secret);

/*
// Convert transaction from and nonce to address.
Address toAddress(Address const& _from, u256 const& _nonce);
*/

// Recovers Public key from signed message hash.
Public recover(Signature const& _sig, core::h256 const& _hash);
	
// Returns siganture of message hash.
Signature sign(Secret const& _k, core::h256 const& _hash);
	
// Verify signature.
bool verify(Public const& _k, Signature const& _s, core::h256 const& _hash);

// Derive key via PBKDF2.
core::bytesSec pbkdf2(std::string const& _pass, core::bytes const& _salt, unsigned _iterations, unsigned _dkLen = 32);

// Derive key via Scrypt.
//core::bytesSec scrypt(std::string const& _pass, core::bytes const& _salt, uint64_t _n, uint32_t _r, uint32_t _p, unsigned _dkLen);

/// Key derivation
core::h256 kdf(Secret const& _priv, core::h256 const& _hash);

/**
 * @brief Generator for non-repeating nonce material.
 * The Nonce class should only be used when a non-repeating nonce
 * is required and, in its current form, not recommended for signatures.
 * This is primarily because the key-material for signatures is 
 * encrypted on disk whereas the seed for Nonce is not. 
 * Thus, Nonce's primary intended use at this time is for networking 
 * where the key is also stored in plaintext.
 */
class Nonce {
public:
	/// Returns the next nonce (might be read from a file).
	static Secret get() { static Nonce s; return s.next(); }

private:
	Nonce() = default;

	/// @returns the next nonce.
	Secret next();

	std::mutex x_value;
	Secret m_value;
};

namespace ecdh {
bool agree(Secret const& _s, Public const& _r, Secret& o_s) noexcept;
}

namespace ecies {
core::bytes kdf(Secret const& _z, core::bytes const& _s1, unsigned kdByteLen);
}

}
