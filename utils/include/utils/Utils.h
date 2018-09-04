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

#include <string>
#include <sstream>
#include <iosfwd>

namespace utils {
/*
 * Json to Proto
 */


/*
 * Proto to Json
 */
/*
template <typename T>
inline std::string toString(T const& t)
{
    std::ostringstream o;
    o << t;
    return o.str();
}

std::string toHex(uint64_t data);
*/

#define MAX_PATH_LENGTH (256)

int MKDIR(std::string const& path);

std::string pathcat(std::string const& path, std::string const& file);

int64_t currentTimestamp();

} // end namespace
