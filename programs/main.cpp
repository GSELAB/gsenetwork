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
#include <chrono>
#include <thread>

#include <core/Address.h>
#include <core/Log.h>
#include <crypto/Common.h>
#include <crypto/GKey.h>
#include <crypto/Valid.h>
#include <chain/Controller.h>
#include <config/Argument.h>
#include <utils/Utils.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

using namespace std;
using namespace crypto;
using namespace chain;
using namespace utils;
namespace {

void printFlag()
{
    std::cout << "\n" <<
        "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG                 GGGSSS                 SSSEEE                 EEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG   GGGGGGGGGGG   GGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG   GGGGGGGGGGGGGGGGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG   GGGGGGGGGGGGGGGGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG   GGGGGGGGGGGGGGGGGSSS                 SSSEEE                 EEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG   GGGGGGG       GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG   GGGGGGGGGGG   GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG   GGGGGGGGGGG   GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGG                 GGGSSS                 SSSEEE                 EEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n";
}

void init(int argc, char **argv)
{
    printFlag();
    toPublic({});
    if (argc > 1 && (strncmp(argv[1], "-p", 2) == 0 || strncmp(argv[1], "-P", 2) == 0)) {
        CINFO << "Set argInstance.m_producerON true";
        ARGs.m_producerON = true;
    }

    initArgument();
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

    controller.exit();
    CINFO << "\n exit by signal  " << sigString;
}

bool shouldExit()
{
    return s_shouldExit;
}

#define SLEEP_SECONDS 2
void doCheck()
{
    std::this_thread::sleep_for(chrono::seconds(SLEEP_SECONDS));
}
}

int main(int argc, char** argv)
{
    init(argc, argv);
    signal(SIGABRT, &exitHandler);
    signal(SIGTERM, &exitHandler);
    signal(SIGINT, &exitHandler);

    GKey key(ARGs.m_secret);
    CINFO << "Secret:" << toHex(key.getSecret().ref());
    CINFO << "Public:" << toHex(key.getPublic().ref());
    CINFO << "Address:" << toHex(key.getAddress().ref());

    auto& genesis = ARGs.m_genesis.m_genesisBlock;
    chain::ChainID chainID = genesis.getBlockHeader().getChainID();
    controller.init(key, chainID);
    while (!shouldExit()) {
        doCheck();
    }

    CINFO <<  "GSE SHUTDOWN";
    return 0;
}

