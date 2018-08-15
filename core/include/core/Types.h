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

/*
 * @author guoygang <hero.gariker@gmail.com>
 * @date 2018
 */

#ifndef __TYPES_HEADER__
#define __TYPES_HEADER__

#include <memory>
#include <vector>
#include <deque>
#include <cstdint>

namespace core {

using                               std::map;
using                               std::vector;
using                               std::unordered_map;
using                               std::string;
using                               std::deque;
using                               std::shared_ptr;
using                               std::weak_ptr;
using                               std::unique_ptr;
using                               std::set;
using                               std::pair;
using                               std::make_pair;
using                               std::enable_shared_from_this;
using                               std::tie;
using                               std::move;
using                               std::forward;
using                               std::to_string;
using                               std::all_of;

}
#endif