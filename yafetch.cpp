#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "config.h"

#include <sys/utsname.h>
#include <sys/sysinfo.h>

struct sysinfo sysinfo_local;
struct utsname uname_local;

std::string get_uptime() {
	long totalsecs = sysinfo_local.uptime;

	int day = totalsecs / (24 * 3600);
	totalsecs %= (24 * 3600);
	int hour = totalsecs / 3600;
	totalsecs %= 3600;	
	int minutes = totalsecs / 60;

	std::stringstream ss;

	if (day==0) {
		if (hour==0) {
			ss << minutes << "m";
		}
		else {
			ss << hour << "h " << minutes << "m";
		}
	}
	else {
		ss << day << "d " << hour << "h " << minutes << "m";
	}
	std::string uptime = ss.str();

	return uptime;
}

std::string get_osname() {
	std::string name;
	std::string line;
	std::ifstream infile;
	size_t pos;
	infile.open("/etc/os-release");
	if (infile.good()) {
		while (infile.good()) {
			std::getline(infile, line);
			pos=line.find("PRETTY_NAME=");
			if(pos!=std::string::npos) {
				name = line;
				break;
			}
		}
	}
	else {
		perror("unable to open /etc/os-release");
		exit(EXIT_FAILURE);
	}
	name.erase(name.begin(), name.begin()+13);
	name.erase(name.end()-1);
	infile.close();
	return name;
}

std::string get_host() {
	std::string product_name;
	std::string product_family;
	std::string space = " ";
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
				product_name = "";
				space = "";
		}
	}
	else {
		perror("unable to get device information");
		exit(EXIT_FAILURE);
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
				product_family = "";
				space = "";
		}
	}
	else {
		perror("unable to get device information");
		exit(EXIT_FAILURE);
	}
	std::stringstream hostss;
	hostss << product_name << space << product_family;
	std::string host = hostss.str();

	if (host.find("OEM")!=std::string::npos ||
		host.find("O.E.M.")!=std::string::npos ||
		host.find("Default")!=std::string::npos ||
		host.find("INVALID")!=std::string::npos ||
		host.find("Not")!=std::string::npos ||
		host.find("System")!=std::string::npos) {
			host = "";
	}

	return host;
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

uint num_files(std::string path) {
	std::filesystem::path pkgfolder = path ;
	using std::filesystem::directory_iterator;
	return std::distance(directory_iterator(pkgfolder), directory_iterator{});
}

std::string get_packages() {
	std::string pkg;
	std::string pkgmgr;

	if (!(shell_cmd("command -v emerge").empty())) {
		pkgmgr = shell_cmd("ls -dL /var/db/pkg/*/* | wc -l 2>&1");
		pkg.append(pkgmgr.begin(), pkgmgr.end()-1);
		pkg.append(" (emerge) ");
	}
	if (!(shell_cmd("command -v pacman").empty())) {
		pkg.append(std::to_string(num_files("/var/lib/pacman/local/")-1));
		pkg.append(" (pacman) ");
	}
	if (!(shell_cmd("command -v apt").empty())) {
		pkgmgr = shell_cmd("dpkg --get-selections | wc -l 2>&1");
		pkg.append(pkgmgr.begin(), pkgmgr.end()-1);
		pkg.append(" (dpkg) ");
	}

        return pkg;
}

std::string get_mem() {
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
	mem << memused << "M / " << memtotal << "M";
	std::string memory = mem.str();

	return memory;
}

std::string get_color() {
	std::string asciicol = logo;
	asciicol.erase(asciicol.begin()+8,asciicol.end());
	return asciicol;
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
	
	std::string username = getlogin();
	std::string sysuptime = get_uptime();
	std::string osname = get_osname();
	std::string hostname = get_host();
	std::string pkgnumber = get_packages();
	std::string sysmemory = get_mem();
	
	std::string color = get_color();
	std::string line;
	std::istringstream f(logo);

	std::getline(f, line);
	std::cout << line << color << username << "@" << uname_local.nodename << "\033[0m\n";
	std::getline(f, line);
	std::cout << line << color << "os:     \033[0m" << osname << "\n";
	std::getline(f, line);
	std::cout << line << color << "host:   \033[0m" << hostname << "\n";
	std::getline(f, line);
	std::cout << line << color << "kernel: \033[0m" << uname_local.release << "\n";
	std::getline(f, line);
	std::cout << line << color << "uptime: \033[0m" << sysuptime << "\n";
	std::getline(f, line);
	std::cout << line << color << "pkgs:   \033[0m" << pkgnumber << "\n";
	std::getline(f, line);
	std::cout << line << color << "memory: \033[0m" << sysmemory << "\n\n";

	exit(0);
}

