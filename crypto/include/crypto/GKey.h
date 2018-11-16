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

#pragma once

#include <core/Common.h>
#include <core/Address.h>
#include <crypto/Common.h>
#include <core/FixedHash.h>

using namespace core;
using namespace crypto;

namespace crypto {

class GKey {
public:
    GKey() {}

    GKey(GKey const& key);

    GKey(Secret const& sec);

    static GKey create();

    static GKey fromEncryptedSeed(bytesConstRef seed, std::string const& password);

    Secret const& getSecret() const;

    Public const& getPublic() const;

    Address const& getAddress() const;

    void setSecret(Secret const& secret);

    bool operator==(GKey const& key) const;

private:
    Secret m_secret;

    Public m_public;

    Address m_address;

};
}