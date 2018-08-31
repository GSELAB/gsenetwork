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

#include "leveldb/db.h"

namespace database {

class DatabaseImpl;

class Database {
public:
    Database();

    Database(std::string& dbFile);

    ~Database();

    void put(std::string& key, std::string& value);

    bool get(std::string& key, std::string* value);

    void del(std::string& key);

private:
    DatabaseImpl *impl;
    leveldb::DB *db;
    leveldb::Options options;
};

}
