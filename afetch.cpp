#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>

#include <sys/utsname.h>
#include <sys/sysinfo.h>
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

std::string get_packages() {
	std::string name = get_osname();
	std::string pkg;
	FILE *stream;
	const int max_buffer = 256;
	char buffer[max_buffer];
	char *cmd = "echo N/A \n";

	if (name.find("Gentoo")!=std::string::npos) {
		cmd = "ls -dL /var/db/pkg/*/* | wc -l 2>&1" ;
	}
	else if (name.find("Mint")!=std::string::npos||name.find("Ubuntu")!=std::string::npos||name.find("Debian")!=std::string::npos) {
		cmd = "dpkg --get-selections | wc -l 2>&1";
	}

	stream = popen(cmd, "r");
        if (stream) {
                while (!feof(stream))
        	if (fgets(buffer, max_buffer, stream) != NULL) pkg.append(buffer);
              	pclose(stream);
        }
        return pkg;

	//else if (osname_pkg == "Ubuntu"
	
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

	std::cout << username << "@" << uname_local.nodename << "\n";
	std::cout << "os:     " << osname << "\n";
	std::cout << "host:   " << hostname << "\n";
	std::cout << "kernel: " << uname_local.release << "\n";
	std::cout << "uptime: " << sysuptime << "\n";
	std::cout << "pkgs:   " << pkgnumber;

	exit(0);
}

