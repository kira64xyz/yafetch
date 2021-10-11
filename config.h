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

#include "yafetch.h"

// Specify which ascii art the program uses
#include "ascii/tux.h"

#define HOSTCOLOR "\033[1;36m"

#define COLOR "\033[1;34m"

// Specify which info to show
std::array<std::string(*)(), 7> info{
  User,
  OSName,
  Host,
  Kernel,
  Packages,
  Uptime,
  Mem,
};
