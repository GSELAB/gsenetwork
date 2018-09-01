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
#include <core/Object.h>

using namespace std;

namespace database {

template<typename T>
class Database {
public:
    Database(string const& file) {
        m_options.create_if_missing = true;
        leveldb::Status status = leveldb::DB::Open(m_options, file, &m_db);
        assert(status.ok());
    }

    ~Database() {
        delete m_db;
    }

    void put(T const& object);

    std::string get(std::string const& key) const;

    void del(std::string const& key);

private:
    leveldb::DB *m_db;
    leveldb::Options m_options;
};

}
