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
#include <thread>
#include <sstream>
#include <iosfwd>
#include <sys/time.h>

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

int64_t currentMillisecond();

int64_t currentMicrosecond();

inline void sleepMilliseconds(int64_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
} // end namespace
