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

#include <core/FixedHash.h>

using namespace core;

namespace crypto {

h256 sha256(bytesConstRef input) noexcept;

h160 ripemd160(bytesConstRef input);
} // end of namespace