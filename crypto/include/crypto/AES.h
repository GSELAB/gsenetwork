#pragma once

#include <crypto/Common.h>

namespace crypto {
    core::bytes aesDecrypt(core::bytesConstRef cipher, std::string const& password, unsigned rounds = 2000, core::bytesConstRef salt = core::bytesConstRef());
} // end namespace crypto