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

#include <stdint.h>
#include <string>
#include <cassert>
#include <time.h>

#ifdef WIN32
error(Not supprot windows)
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <core/Log.h>
#include <utils/Utils.h>

namespace utils {


std::string toHex(uint64_t data)
{
    return std::string();
}



int MKDIR(std::string const& path)
{
    char dir[MAX_PATH_LENGTH] = { 0 };
    uint32_t len = path.length();

    if (len > MAX_PATH_LENGTH) {
        CERROR << "SIZEOF(" << path << ") > " << MAX_PATH_LENGTH;
        return -1;
    }

    for (unsigned i = 0; i < len; i++) {
        dir[i] = path[i];
        if (dir[i] == '/' || dir[i] == '\\') {
            if (access(dir, 0) != 0) {
                int ret = mkdir(dir, S_IRWXU);
                if (ret != 0) {
                    CERROR << "MKDIR " << dir << " failed!";
                    return -1;
                }
            }
        }
    }

    return 0;
}

std::string pathcat(std::string const& path, std::string const& file)
{
    size_t len = path.length();

    size_t pos = file.find('/');
    if (pos != std::string::npos)
        CERROR << "Invalid file name!";
    assert(pos == std::string::npos);

    if (path[len - 1] == '/') {
        return path + file;
    } else {
        return path + "/" + file;
    }

}

/*
 * @return seconds from 1979.01.01 to now
 * @ ctime(&(time_t)tt);
 */
int64_t currentTimestamp()
{
    int64_t curr;
    curr = static_cast<int64_t>(time(NULL));
    return curr;
}

} // end namespace
