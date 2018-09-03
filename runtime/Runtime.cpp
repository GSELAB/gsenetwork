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

using namespace runtime;
using namespace runtime::storage;

Runtime::Runtime(Transaction const& transaction, Block const& block, std::shared_ptr<Repository> repository): m_transaction(transaction)
{
    m_repository = repository;
}

void Runtime::init()
{

}

void Runtime::excute()
{
    Address const& sender = m_transaction.getSender();
    Address const& recipient = m_transaction.getRecipient();
    bytes const& data = m_transaction.getData();
    uint64_t value = m_transaction.getValue();

    // transafer
    m_repository->transfer(sender, recipient, value);

}

void Runtime::finished()
{

}

uint64_t Runtime::getResult() const
{
    return 0;
}
