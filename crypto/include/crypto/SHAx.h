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

#include <stdint.h>
#include <stddef.h>

namespace crypto {

void sha3_256(uint8_t* result, uint8_t const* input, size_t length);

void sha3_512(uint8_t* result, uint8_t const* input, size_t length);

}
