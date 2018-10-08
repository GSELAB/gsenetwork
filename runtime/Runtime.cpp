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

#include <runtime/Runtime.h>
#include <core/Address.h>
#include <storage/Repository.h>
#include <runtime/action/ActionFactory.h>

namespace runtime {

using namespace action;

Runtime::Runtime(Transaction const& transaction, Block const& block, std::shared_ptr<storage::Repository> repo):
    m_transaction(transaction), m_repo(repo), m_type(NormalType)
{
    m_block = (Block*)&block;
}

Runtime::Runtime(Transaction const& transaction, std::shared_ptr<storage::Repository> repo):
    m_transaction(transaction), m_repo(repo), m_type(PreType)
{
    m_block = nullptr;
}

void Runtime::init()
{

}

void Runtime::excute()
{
    try {
        ActionFactory factory(m_transaction, m_block, m_repo);
        factory.init();
        factory.execute();
        factory.finalize();
    } catch (Exception e) {

    }
}

void Runtime::finished()
{
    if (m_type == NormalType) {
        // commit data
    }
}

uint64_t Runtime::getResult() const
{
    return 0;
}

}