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

#include <boost/thread.hpp>

#include <chain/Controller.h>
#include <core/Log.h>

using namespace database;
using namespace net;

namespace chain {

namespace {
void my_func()
{
    CINFO << "GSE TEST!";
}
}

void Controller::init()
{
    CINFO << "Start database init...";
    m_dbc = new DatabaseController();
    m_dbc->init();

    CINFO << "Start network init...";
    m_net = new NetController();
    m_net->init();


    CINFO << "Start test init...";
    boost::thread t(my_func);
    t.join();
}

std::shared_ptr<TransactionReceipt> Controller::processTransaction(Transaction const& transaction, int64_t max_timestamp)
{
    return nullptr;
}

void Controller::processBlock(Block const& block)
{

}

void Controller::processTransaction(Transaction const& transaction)
{

}

chain::ChainID Controller::getChainID() const
{
    return m_chainID;
}

void Controller::setChainID(chain::ChainID chainID)
{
    m_chainID = chainID;
}

// @only used by rpc module
bool Controller::generateTransaction()
{
    return true;
}

// @only used by rpc module
bool Controller::addTransaction(Transaction const& transaction)
{
    return true;
}




Controller controller(DEFAULT_GSE_NETWORK);

} /* end namespace */