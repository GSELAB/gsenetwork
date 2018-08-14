#include <cassert>

#include "database/Database.h"

namespace database {

Database::Database(std::string &db_file)
{
    m_options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(m_options, db_file, &m_db);
    assert(status.ok());
}

Database::~Database()
{
    delete m_db;
}

void Database::put(std::string &key, std::string &value)
{
    leveldb::Status status = db->Put(leveldb::WriteOptions(), key, &value);
    assert(status.ok());
}

bool Database::get(std::string &key, std::string *value)
{
    leveldb::Status status = db->Get(leveldb::ReadOptions(), key, value);
    // assert(status.ok());
    return true;
}

void Database::delete(std::string &key)
{
    leveldb::Status status = db->Delete(leveldb::WriteOptions(), key);
}

} // endof namespace