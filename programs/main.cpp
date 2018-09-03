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

#include <core/Address.h>
#include <core/Log.h>
#include <crypto/Common.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include "chain/Controller.h"
#include <chrono>
#include <thread>


using namespace std;
using namespace crypto;
using namespace chain;

namespace {

void init(int argc, char **argv)
{
    // init secp256k1 context

    if (argc == 1) {
        std::cout << "one param:" << argv[0] << "\n";
    }

}

bool s_shouldExit = false;
void exitHandler(int sig)
{
    std::string sigString;
    s_shouldExit = true;
    switch (sig) {
        case SIGABRT:
            sigString = "SIGABRT";
            break;
        case SIGTERM:
            sigString = "SIGTERM";
            break;
        case SIGINT:
            sigString = "SIGINT";
            break;
        default:
            sigString = "UNKNOWN";
            break;
    }

    std::cout << "\nexit by signal  " << sigString << "\n";

}

bool shouldExit()
{
    return s_shouldExit;
}

#define SLEEP_SECONDS 2

void doCheck()
{
    //boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(10));
    //this_thread::sleep_for(chrono::milliseconds(100));
    std::cout << "do check! sleep " << SLEEP_SECONDS << "\n";
    std::this_thread::sleep_for(chrono::seconds(SLEEP_SECONDS));
    //this_thread::sleep_for(chrono::milliseconds(100));
}

}


int main(int argc, char** argv)
{
    //boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(100));

    /*
    string localIP;
    unsigned localPort = 25050;

    bool enableProducer = false;
    Address producerAddress;
    Secret secret;
    {
        // init secret
    }
    Public pubK;
    */

    init(argc, argv);



    signal(SIGABRT, &exitHandler);
    signal(SIGTERM, &exitHandler);
    signal(SIGINT, &exitHandler);

    // TODO : Controller
    CINFO << "Controller->init\n";
    chain::toController()->init();

    if (true) {

        while (!shouldExit()) {
            doCheck();
        }
    }

    std::cout << "<-- GSENetwork shutdown -->" << std::endl;
    return 0;
}

