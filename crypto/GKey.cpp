/*
 * Copyright (c) 2018 GSENetwork
 *
 * This file is part of GSENetwork.
 *
 * GSENetwork is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 */

#include <crypto/GKey.h>
#include <crypto/AES.h>
#include <crypto/SHA3.h>
#include <core/Log.h>

using namespace crypto;

namespace crypto {

GKey::GKey(GKey const& key)
{
    m_secret = key.getSecret();
    m_public = key.getPublic();
    m_address = key.getAddress();
}

GKey::GKey(Secret const& sec)  : m_secret(sec), m_public(toPublic(sec))
{
    if (m_public) {
        m_address = toAddress(m_public);
    }
}

GKey GKey::create()
{
    while (true) {
        GKey _key(Secret::random());
        if (_key.getAddress())
            return _key;
    }
}

GKey GKey::fromEncryptedSeed(bytesConstRef seed, std::string const& password)
{
    return GKey(Secret(sha3(aesDecrypt(seed, password))));
}

Secret const& GKey::getSecret() const
{
    return m_secret;
}

Public const& GKey::getPublic() const
{
    return m_public;
}

Address const& GKey::getAddress() const
{
    return m_address;
}

void GKey::setSecret(Secret const& secret)
{
    m_secret = secret;
    m_public = toPublic(m_secret);
    m_address = toAddress(m_public);
}

bool GKey::operator==(GKey const& key) const
{
    return m_secret == key.getSecret();
}
}