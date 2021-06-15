// Specify which ascii art the program uses
#include "ascii/tux.h"

#define HOSTCOLOR "\033[1;34m"

#define COLOR "\033[1;36m"

// Specify which info to show
const std::string (*info[])() = {
	get_user,
	get_osname,
	get_host,
	get_kernel,
	get_packages,
	get_uptime,
	get_mem,
};

