// Specify which ascii art the program uses
#include "ascii/tux.h"

#define HOSTCOLOR "\033[1;34m"

#define COLOR "\033[1;36m"

// Specify which info to show
const std::string (*info[])() = {
  User,
  OSName,
  Host,
  Kernel,
  Packages,
  Uptime,
  Mem,
};

