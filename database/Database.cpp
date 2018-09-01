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

using namespace std;

namespace database {

template<typename T>
void Database<T>::put(T const& object)
{
    // key <- std::string : value <- std::string
    leveldb::Status status = m_db->Put(leveldb::WriteOptions(), object.getKey(), object.getRLPData());
    assert(status.ok());
}

template<typename T>
std::string Database<T>::get(std::string const& key) const
{
    // key <- std::string : value <- std::string
    std::string value;
    leveldb::Status status = m_db->Get(leveldb::ReadOptions(), key, &value);
    assert(status.ok());
    return value;
}

template<typename T>
void Database<T>::del(std::string const& key)
{
    leveldb::Status status = m_db->Delete(leveldb::WriteOptions(), key);
}

} // endof namespace