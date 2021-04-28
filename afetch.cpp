#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <linux/kernel.h>

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
			ss << minutes << " minutes";
		}
		else {
			ss << hour << " hours " << minutes << " minutes";
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
	std::ifstream infile;
	infile.open("/etc/os-release");
	if (infile.good()) {
		std::getline(infile, name);
	}
	else {
		perror("unable to open /etc/os-release");
		exit(EXIT_FAILURE);
	}
	name.erase(name.begin(), name.begin()+5);
	infile.close();
	return name;
}

std::string get_host() {
	std::string product_name;
	std::string product_family;
	std::ifstream infile;
	infile.open("/sys/devices/virtual/dmi/id/product_name");
	if (infile.good()) {
		std::getline(infile, product_name);
	}
	else {
		perror("unable to get device information");
		exit(EXIT_FAILURE);
	}
	infile.close();
	infile.open("/sys/devices/virtual/dmi/id/product_family");
	if (infile.good()) {
		std::getline(infile, product_family);
	}
	else {
		perror("unable to get device information");
                exit(EXIT_FAILURE);
	}
	std::stringstream sa;
	sa << product_name << " " << product_family;
	std::string host = sa.str();
	return host;
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

	std::cout << username << "@" << uname_local.nodename << "\n";
	std::cout << "os: \t\t\t" << osname << "/" << uname_local.sysname << "\n";
	std::cout << "host: \t\t\t" << hostname << "\n";
	std::cout << "kernel: \t\t" << uname_local.release << "\n";
	std::cout << "uptime: \t\t" << sysuptime << "\n";

	exit(0);
}

