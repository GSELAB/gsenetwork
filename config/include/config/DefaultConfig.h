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

#include <config/Config.h>

namespace config {

class DefaultConfig: public Config {
public:
    std::string getDBPath();

private:
    std::string m_dbPath;

};
}


