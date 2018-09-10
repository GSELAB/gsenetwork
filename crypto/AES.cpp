#include <crypto/AES.h>

#include <cryptopp/aes.h>
#include <cryptopp/sha.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>

using namespace core;

namespace crypto {
bytes aesDecrypt(bytesConstRef cipher, std::string const& password, unsigned rounds, bytesConstRef salt)
{
    bytes pw = asBytes(password);

    if (salt.size() == 0) {
        salt = &pw;
    }

    bytes target(64);
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256>().DeriveKey(target.data(), target.size(), 0, pw.data(), pw.size(), salt.data(), salt.size(), rounds);
    try {
        CryptoPP::AES::Decryption aesDecryption(target.data(), 16);
        auto _cipher = cipher.cropped(16);
        auto iv = cipher.cropped(0, 16);
        CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv.data());
        std::string decrypted;
        CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decrypted));
        stfDecryptor.Put(_cipher.data(), _cipher.size());
        stfDecryptor.MessageEnd();
        return asBytes(decrypted);
    } catch (std::exception const& e) {
        THROW_GSEXCEPTION(e.what());
    }
}

}