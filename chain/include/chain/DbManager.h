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

#include "database/Database.h"

namespace chain {

class DbManager {
public:
    DbManager();

    virtual ~DbManager();


private:
    database::Database   m_database;
    database::Database   m_reversibleBlocks;

    OSODatabase         m_osoDatabase;

};


} // end of namespace