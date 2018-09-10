#include <string>

#include <crypto/Encrypt.h>
#include <crypto/Secp256k1PP.h>
#include <cryptopp/sha.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/pwdbased.h>

using namespace std;

namespace crypto {

void encrypt(Public const& pubKey, bytesConstRef plain, bytes& o_cipher)
{
	bytes io = plain.toBytes();
	Secp256k1PP::get()->encrypt(pubKey, io);
	o_cipher = std::move(io);
}

void encryptECIES(Public const& pubKey, bytesConstRef plain, bytes& o_cipher)
{
	encryptECIES(pubKey, bytesConstRef(), plain, o_cipher);
}

void encryptECIES(Public const& pubKey, bytesConstRef sharedMacData, bytesConstRef plain, bytes& o_cipher)
{
	bytes io = plain.toBytes();
	Secp256k1PP::get()->encryptECIES(pubKey, sharedMacData, io);
	o_cipher = std::move(io);
}

void encryptSym(Secret const& secret, bytesConstRef plain, bytes& o_cipher)
{
	// TODO: @alex @subtly do this properly.
	encrypt(toPublic(secret), plain, o_cipher);
}

std::pair<bytes, h128> encryptSymNoAuth(SecureFixedHash<16> const& k, bytesConstRef plain)
{
	h128 iv(Nonce::get().makeInsecure());
	return make_pair(encryptSymNoAuth(k, iv, plain), iv);
}

bytes encryptAES128CTR(bytesConstRef k, h128 const& iv, bytesConstRef plain)
{
	if (k.size() != 16 && k.size() != 24 && k.size() != 32) return bytes();
	CryptoPP::SecByteBlock key(k.data(), k.size());
	try {
		CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption e;
		e.SetKeyWithIV(key, key.size(), iv.data());
		bytes ret(plain.size());
		e.ProcessData(ret.data(), plain.data(), plain.size());
		return ret;
	} catch (CryptoPP::Exception& _e) {
		cerr << _e.what() << endl;
		return bytes();
	}
}

} // namespace crypto