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

#include <cassert>

#include "database/Database.h"
#include "leveldb/db.h"

namespace database {

class DatabaseImpl {

private:
    leveldb::DB *db;
    leveldb::Options options;
};

Database::Database()
{

}

Database::Database(std::string &dbFile)
{
    /*
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, dbFile, &db);
    assert(status.ok());
    */
}

Database::~Database()
{
    //delete db;
}

void Database::put(std::string &key, std::string &value)
{
    /*
    leveldb::Status status = db->Put(leveldb::WriteOptions(), key, value);
    assert(status.ok());
    */
}

bool Database::get(std::string& key, std::string* value)
{
    /*
    leveldb::Status status = db->Get(leveldb::ReadOptions(), key, value);
    // assert(status.ok());
    return true;
    */
    return true;
}

void Database::del(std::string& key)
{
    //leveldb::Status status = db->Delete(leveldb::WriteOptions(), key);
}

} // endof namespace