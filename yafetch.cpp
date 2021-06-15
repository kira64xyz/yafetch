#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#include "yafetch.h"
#include "config.h"

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>

struct sysinfo sysinfo_local;
struct utsname uname_local;

const std::string get_uptime() {
	long totalsecs = sysinfo_local.uptime;

	int day = totalsecs / (24 * 3600);
	totalsecs %= (24 * 3600);
	int hour = totalsecs / 3600;
	totalsecs %= 3600;	
	int minutes = totalsecs / 60;

	std::stringstream uptime;

	if (day==0) {
		if (hour==0) {
			uptime << COLOR << "uptime:\t\033[0m" << minutes << "m\n";
		}
		else {
			uptime << COLOR << "uptime:\t\033[0m" << hour << "h " << minutes << "m\n";
		}
	}
	else {
		uptime << COLOR << "uptime:\t\033[0m" << day << "d " << hour << "h " << minutes << "m\n";
	}

	return uptime.str();
}

const std::string get_osname() {
	std::string line;
	std::ifstream infile;
	size_t pos;
	infile.open("/etc/os-release");
	if (infile.good()) {
		while (infile.good()) {
			std::getline(infile, line);
			pos=line.find("PRETTY_NAME=");
			if(pos!=std::string::npos) {
				break;
			}
		}
	}
	else {
		perror("unable to open /etc/os-release");
		exit(EXIT_FAILURE);
	}
	line.erase(line.begin(), line.begin()+13);
	line.erase(line.end()-1);
	infile.close();
	std::stringstream name;
	name << COLOR << "os:\033[0m\t" << line << "\n";
	return name.str();
}

const std::string get_host() {
	std::string product_name;
	std::string product_family;
	std::ifstream infile;
	infile.open("/sys/devices/virtual/dmi/id/product_name");
	if (infile.good()) {
		std::getline(infile, product_name);
		if (product_name.find("OEM")!=std::string::npos ||
			product_name.find("O.E.M.")!=std::string::npos ||
			product_name.find("Default")!=std::string::npos ||
			product_name.find("INVALID")!=std::string::npos ||
			product_name.find("Not")!=std::string::npos ||
			product_name.find("System")!=std::string::npos) {
				infile.close();
				infile.open("/sys/devices/virtual/dmi/id/board_name");
				if (infile.good()) {
					std::getline(infile, product_name);
				}
				else {
					perror("unable to get device information");
                			exit(EXIT_FAILURE);
				}

		}
	}
	infile.close();
	infile.open("/sys/devices/virtual/dmi/id/product_family");
	if (infile.good()) {
		std::getline(infile, product_family);
		if (product_family.find("OEM")!=std::string::npos ||
			product_family.find("O.E.M.")!=std::string::npos ||
			product_family.find("Default")!=std::string::npos ||
			product_family.find("INVALID")!=std::string::npos ||
			product_family.find("Not")!=std::string::npos ||
			product_family.find("System")!=std::string::npos) {
				product_family.clear();
		}
	}
	infile.close();
	std::stringstream host;
	host << COLOR << "host:\t\033[0m" << product_name << " " << product_family << "\n";

	return host.str();
}

std::string shell_cmd(const char *icmd) {
	FILE *stream;
	const int max_buffer = 256;
        char buffer[max_buffer];
	std::string cmdo;
	stream = popen(icmd, "r");

        if (stream) {
                while (!feof(stream))
                                if (fgets(buffer, max_buffer, stream) != NULL) cmdo.append(buffer);
        				pclose(stream);
        }
	return cmdo;
}

uint num_files_pacman(std::string path) {
	std::filesystem::path pkgfolder = path;
	using std::filesystem::directory_iterator;
	return std::distance(directory_iterator(pkgfolder), directory_iterator{});
}

uint num_files_portage(std::string path) {
        std::filesystem::path pkgfolder = path;
        uint total_subdirs = 0;
        using std::filesystem::recursive_directory_iterator;
        for(auto i = recursive_directory_iterator(path) ; i!= recursive_directory_iterator() ; ++i) {
          if(i.depth() == 1) {
            i.disable_recursion_pending();
            total_subdirs++;
          }
        }

	return total_subdirs;
}

const std::string get_packages() {
	std::string pkg;
	std::string pkgmgr;
	struct stat s;

	if (stat("/etc/portage", &s) == 0) {
		pkg.append(std::to_string(num_files_portage("/var/db/pkg")));
		pkg.append(" (emerge) ");
	}
	if (stat("/etc/pacman.d", &s) == 0) {
		pkg.append(std::to_string(num_files_pacman("/var/lib/pacman/local/")-1));
		pkg.append(" (pacman) ");
	}
	if (stat("/etc/apt", &s) == 0) {
		pkgmgr = shell_cmd("dpkg --get-selections | wc -l 2>&1");
		pkg.append(pkgmgr.begin(), pkgmgr.end()-1);
		pkg.append(" (dpkg) ");
	}
	if (stat("/nix", &s) == 0) {
		if (stat("/etc/nix", &s) == 0) {
			pkgmgr = shell_cmd("nix-store --query --requisites /run/current-system | wc -l");
		} else {
			pkgmgr = shell_cmd("nix-env -q | wc -l");
		}
		pkg.append(pkgmgr.begin(), pkgmgr.end()-1);
		pkg.append(" (nix) ");
        }

	std::stringstream pkgs;
	pkgs << COLOR << "pkgs:\t\033[0m" << pkg << "\n";
	
        return pkgs.str();
}

const std::string get_mem() {
	unsigned long memtotal = sysinfo_local.totalram / 1024;
	unsigned long memavail;
	std::string memavailstr;
	std::string searchtoken;
	size_t mpos;
	std::ifstream infile("/proc/meminfo");
	if (infile.good()) {
		while (infile.good()) {
			std::getline(infile, searchtoken);
			mpos=searchtoken.find("MemAvailable");
			if(mpos!=std::string::npos) {
				memavailstr = searchtoken;
					break;
			}
		}
	}
	else {
		perror("unable to open /proc/meminfo");
		exit(EXIT_FAILURE);
	}
	memavailstr.erase(memavailstr.begin(), memavailstr.begin()+13);
	infile.close();
	std::istringstream mema (memavailstr);
	mema >> memavail;
	if (mema.fail()) {
		perror("unable to calculate ram usage");
		exit(EXIT_FAILURE);
	}

	unsigned long memused = memtotal - memavail;
	memused /= 1024;
	memtotal /= 1024;
	std::stringstream mem;
	mem << COLOR << "memory:\t\033[0m" << memused << "M / " << memtotal << "M\n";

	return mem.str();
}

const std::string get_user() {
	std::stringstream su;
	su << HOSTCOLOR << getlogin() << "@" << uname_local.nodename << "\033[0m\n";
	return su.str();
}

const std::string get_kernel() {
	std::stringstream sk;
	sk << COLOR << "kernel:\t\033[0m" << uname_local.release << "\033[0m\n";
	return sk.str();
}

int main() {
	if(uname(&uname_local) != 0) {
		perror("uname error");
		exit(EXIT_FAILURE);
	}
	if (sysinfo(&sysinfo_local) != 0) {
		perror("sysinfo error");
		exit(EXIT_FAILURE);
	}
	
	std::string line;
	std::istringstream f(logo);
	std::string inf;

	for (int x=0;x<7;x++){
		std::getline(f, line);
		inf.append(line);
		inf.append((*info[x])());
	}
	puts(inf.c_str());

	exit(0);
}
