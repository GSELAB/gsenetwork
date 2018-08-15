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

#ifndef __BUNDLE_BUNDLE_HEADER__
#define __BUNDLE_BUNDLE_HEADER__

namespace bundle {

class Bundle {
public:
    virtual char *getData();
};
} // end of namespace bundle
#endif