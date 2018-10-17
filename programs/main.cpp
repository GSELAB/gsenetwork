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
#include <chain/Controller.h>
#include <config/Argument.h>


#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

using namespace std;
using namespace crypto;
using namespace chain;

namespace {

void printFlag()
{
    std::cout << "\n" <<
        "\t\tGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGG                 GGGSSS                 SSSEEE                 EEEEEE\n" <<
        "\t\tGGGGGG   GGGGGGGGGGG   GGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGG   GGGGGGGGGGGGGGGGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGG   GGGGGGGGGGGGGGGGGSSS   SSSSSSSSSSSSSSSSSEEE   EEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGG   GGGGGGGGGGGGGGGGGSSS                 SSSEEE                 EEEEEE\n" <<
        "\t\tGGGGGG   GGGGGGG       GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGG   GGGGGGGGGGG   GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGG   GGGGGGGGGGG   GGGSSSSSSSSSSSSSSSSS   SSSEEE   EEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGG           GGG   GGGSSS                 SSSEEE                 EEEEEE\n" <<
        "\t\tGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEE\n" <<
        "\t\tGGGGGGGGGGGGGGGGGGGGGGGGGGSSSSSSSSSSSSSSSSSSSSSSSEEEEEEEEEEEEEEEEEEEEEEEEEE\n\n";
}

void init(int argc, char **argv)
{
    printFlag();
    toPublic({});
    if (argc > 1 && (strncmp(argv[1], "-p", 2) == 0 || strncmp(argv[1], "-P", 2) == 0)) {
        CINFO << "Set argInstance.m_producerON true";
        argInstance.m_producerON = true;
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

    controller.exit();
    CINFO << "\nexit by signal  " << sigString;

}

bool shouldExit()
{
    return s_shouldExit;
}

#define SLEEP_SECONDS 2
void doCheck()
{
    //boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(10));
    std::this_thread::sleep_for(chrono::seconds(SLEEP_SECONDS));
}
} // namespace

int main(int argc, char** argv)
{
    //setDefaultOrCLocale();
    //bool enableProducer = false;
    //Address producerAddress;
    init(argc, argv);
    signal(SIGABRT, &exitHandler);
    signal(SIGTERM, &exitHandler);
    signal(SIGINT, &exitHandler);

    Secret sec("4077db9374f9498aff4b4ae6eb1400755655b50457930193948d2dc6cf70bf0f");
    GKey key(sec);
    CINFO << "Secret:" << toHex(key.getSecret().ref());
    CINFO << "Public:" << toHex(key.getPublic().ref());
    CINFO << "Address:" << toHex(key.getAddress().ref());

    // TODO : Controller
    controller.init(key);
    while (!shouldExit())
        doCheck();

    CINFO <<  "GSE SHUTDOWN";
    return 0;
}

