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

#include <leveldb/db.h>
#include <database/Database.h>
#include <core/Log.h>
#include <utils/Utils.h>

using namespace std;

namespace database {

class DatabaseImpl {
public:
    DatabaseImpl(std::string const& file) {
        m_options.create_if_missing = true;
        leveldb::Status status = leveldb::DB::Open(m_options, file, &m_db);
        if (status.ok() == false) {
            CERROR << "Unable to open/create database " << file;
        } else {
            CINFO << "Open/create " << file << " success!";
        }
        assert(status.ok());
    }

    ~DatabaseImpl() {
        delete m_db;
    }

    void put(std::string const& key, std::string const& value) {
        leveldb::Status status = m_db->Put(leveldb::WriteOptions(), key, value);
        assert(status.ok());
    }

    std::string get(std::string const& key) const {
        std::string value;
        leveldb::Status status = m_db->Get(leveldb::ReadOptions(), key, &value);
        assert(status.ok());
        return value;
    }

    void del(std::string const& key) {
        leveldb::Status status = m_db->Delete(leveldb::WriteOptions(), key);
    }

private:
    leveldb::DB *m_db;
    leveldb::Options m_options;
};

Database::Database(std::string const& file)
{
    utils::MKDIR(file);
    m_impl = std::unique_ptr<DatabaseImpl>(new DatabaseImpl(file));
}

Database::Database(std::string const& path, std::string const& file)
{
    std::string pathFile = utils::pathcat(path, file);
    utils::MKDIR(path);
    m_impl = std::unique_ptr<DatabaseImpl>(new DatabaseImpl(pathFile));
}

Database::~Database()
{

}

void Database::put(core::Object& object)
{
    m_impl->put(object.getKey(), object.getRLPData());
    //m_impl->put(*(std::string const*)&object.getKey(), *(std::string const*)&object.getRLPData());
}

std::string Database::get(std::string const& key) const
{
    return m_impl->get(key);
}

void Database::del(std::string const& key)
{
    m_impl->del(key);
}

} // endof namespace