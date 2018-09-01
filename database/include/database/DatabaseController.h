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

#pragma once

#include <chain/Controller.h>

using namespace chain;

namespace database {

class DatabaseController {
public:
    DatabaseController();

    DatabaseController(Controller *controller);

private:

    Controller *controller;
};
} // namespace end