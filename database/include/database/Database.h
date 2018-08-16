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

/*
 * @author guoygang <hero.gariker@gmail.com>
 * @date 2018
 */

#pragma once

#include <string>

#include "leveldb/db.h"

namespace database {

class Database {
public:
    Database(std::string &db_file);

    virtual ~Database();

    void put(std::string &key, std::string &value);

    bool get(std::string &key, std::string *value);

    void delete(std::string &key);



private:
    leveldb::DB *m_db;
    leveldb::Options m_options;



};

}
