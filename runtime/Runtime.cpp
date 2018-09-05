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

Runtime::Runtime(Transaction const& transaction, Block const& block): m_transaction(transaction)
{

}

Runtime::Runtime(Transaction const& transaction)
{

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
