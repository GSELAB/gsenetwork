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

#include <iostream>

#include <boost/version.hpp>
#include <boost/timer.hpp>
#include "chain/DbManager.h"
#include "chain/SubChainController.h"
#include "chain/Controller.h"

using namespace std;

int main(int argc, char **argv)
{
    boost::timer t;
    cout << "max timespan: " << t.elapsed_max() / 3600 << "h" << endl;
    cout << "min timespan: " << t.elapsed_min() << "s" << endl;
    cout << "now time elapsed: " << t.elapsed() << "s" << endl;
    cout << "boost version" << BOOST_VERSION <<endl;
    cout << "boost lib version" << BOOST_LIB_VERSION <<endl;
    std::cout << "GSENetwork Interview!" << std::endl;

    //std::cout << "GSENetwork protocol!" << std::endl;
    return 0;
}

