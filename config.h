/* Copyright (C) 2021 Ashley Chiara <ashley@kira64.xyz>
This file is part of yafetch.

yafetch is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

yafetch is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with yafetch; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>. */

#include <string>

struct Colors {
    std::string color = "\033[1;34m";
    std::string hostcolor = "\033[1;36m";

    std::string wrap(std::string str) {
        return color + str + "\033[0m";
    }

    std::string wraphost(std::string str) {
        return hostcolor + str + "\033[0m";
    }
};
