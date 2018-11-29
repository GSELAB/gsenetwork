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

#include <runtime/common/Runtime.h>
#include <core/Address.h>
#include <storage/Repository.h>
#include <runtime/action/ActionFactory.h>

namespace runtime {

using namespace action;
namespace common {
void Runtime::init()
{

}

void Runtime::excute()
{
    ActionFactory factory(m_transaction, m_block, m_repo);
    factory.init();
    factory.execute();
    factory.finalize();
    m_repo->put(m_transaction);
}

void Runtime::finished()
{

}

uint64_t Runtime::getResult() const
{
    return 0;
}
}
}