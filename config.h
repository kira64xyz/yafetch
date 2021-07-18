#include "yafetch.h"

// Specify which ascii art the program uses
#include "ascii/tux.h"

#define HOSTCOLOR "\033[1;34m"

#define COLOR "\033[1;36m"

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
