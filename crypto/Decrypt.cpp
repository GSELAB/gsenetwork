#include <string>

#include <crypto/Decrypt.h>
#include <crypto/Secp256k1PP.h>
#include <cryptopp/sha.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/pwdbased.h>

using namespace std;

namespace crypto {

bool decrypt(Secret const& secret, bytesConstRef cipher, bytes& o_plaintext)
{
	bytes io = cipher.toBytes();
	Secp256k1PP::get()->decrypt(secret, io);
	if (io.empty()) return false;
	o_plaintext = std::move(io);
	return true;
}

bool decryptECIES(Secret const& secret, bytesConstRef cipher, bytes& o_plaintext)
{
	return decryptECIES(secret, bytesConstRef(),  cipher, o_plaintext);
}

bool decryptECIES(Secret const& secret, bytesConstRef sharedMacData, bytesConstRef _cipher, bytes& o_plaintext)
{
	bytes io = _cipher.toBytes();
	if (!Secp256k1PP::get()->decryptECIES(secret, sharedMacData, io)) return false;
	o_plaintext = std::move(io);
	return true;
}

bool decryptSym(Secret const& secret, bytesConstRef cipher, bytes& o_plain)
{
	// TODO: @alex @subtly do this properly.
	return decrypt(secret, cipher, o_plain);
}

bytesSec decryptAES128CTR(bytesConstRef k, h128 const& iv, bytesConstRef cipher)
{
	if (k.size() != 16 && k.size() != 24 && k.size() != 32) return bytesSec();
	CryptoPP::SecByteBlock key(k.data(), k.size());
	try {
		CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(key, key.size(), iv.data());
		bytesSec ret(cipher.size());
		d.ProcessData(ret.writable().data(), cipher.data(), cipher.size());
		return ret;
	} catch (CryptoPP::Exception& _e) {
		cerr << _e.what() << "\n";
		return bytesSec();
	}
}
} // namespace crypto