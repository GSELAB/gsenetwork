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
#include <vector>

#include <leveldb/db.h>
#include <database/Database.h>
#include <core/Log.h>
#include <core/Producer.h>
#include <utils/Utils.h>

using namespace std;
using namespace core;

namespace database {

class DatabaseImpl {
public:
    DatabaseImpl(std::string const& file) {
        m_options.create_if_missing = true;
        leveldb::Status status = leveldb::DB::Open(m_options, file, &m_db);
        if (!status.ok()) {
            CERROR << "Unable to open/create database " << file;
            BOOST_THROW_EXCEPTION(std::range_error("Error to call db->get()"));
        }
    }

    ~DatabaseImpl() {
        delete m_db;
    }

    void put(bytes const& key, bytes const& value) {
        leveldb::Status status = m_db->Put(leveldb::WriteOptions(), toString(key), toString(value));
        assert(status.ok());
    }

    bytes get(bytes const& key) const {
        std::string value;
        leveldb::Status status = m_db->Get(leveldb::ReadOptions(), toString(key), &value);
        if (status.ok()) {
            return toBytes(value);
        } else if (status.IsNotFound()) {
            return EmptyBytes;
        }

        BOOST_THROW_EXCEPTION(std::range_error("Error to call db->get()"));
    }

    void del(bytes const& key) {
        leveldb::Status status = m_db->Delete(leveldb::WriteOptions(), toString(key));
    }

    std::vector<Producer> getProducerList() const {
        std::vector<Producer> producerList;
        leveldb::Iterator* itr = m_db->NewIterator(leveldb::ReadOptions());
        for (itr->SeekToFirst(); itr->Valid(); itr->Next())
        {
            std::string value = itr->value().ToString();
            bytes bytesValue = toBytes(value);
            producerList.push_back(Producer(bytesConstRef(&bytesValue)));
        }
        return producerList;
    }

private:
    leveldb::DB *m_db;
    leveldb::Options m_options;
};

Database::Database(std::string const& file)
{
    utils::MKDIR(file);
    m_impl = new DatabaseImpl(file);
}

Database::Database(std::string const& path, std::string const& file)
{
    std::string pathFile = utils::pathcat(path, file);
    utils::MKDIR(path);
    m_impl = new DatabaseImpl(pathFile);
}

Database::~Database()
{
    delete m_impl;
}

void Database::put(core::Object& object)
{
    bytes key = object.getKey();
    bytes value = object.getRLPData();
    m_impl->put(key, value);
}

bytes Database::get(bytes const& key) const
{
    return m_impl->get(key);
}

void Database::del(bytes const& key)
{
    m_impl->del(key);
}

std::vector<Producer> Database::getProducerList() const
{
    return m_impl->getProducerList();
}
} // endof namespace
