#ifndef __DATABASE_HEADER__
#define __DATABASE_HEADER__

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

#endif