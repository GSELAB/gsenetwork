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

#include <core/Object.h>
#include <core/CommonIO.h>
#include <core/Common.h>
#include <core/Producer.h>

namespace database {

class DatabaseImpl;

class Database {
public:
    Database(std::string const& file);

    Database(std::string const& path, std::string const& file);

    ~Database();

    void put(core::Object& object);

    core::bytes get(core::bytes const& key) const;

    void del(core::bytes const& key);

    std::vector<core::Producer> getProducerList() const;
private:
    DatabaseImpl* m_impl;
};

} // end namespace
