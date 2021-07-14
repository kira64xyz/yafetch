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

struct sysinfo sysinfoLocal;
struct utsname unameLocal;

const std::string Uptime() {
  long totalSecs = sysinfoLocal.uptime;

  int day = totalSecs / (24 * 3600);
  totalSecs %= (24 * 3600);
  int hour = totalSecs / 3600;
  totalSecs %= 3600;	
  int minutes = totalSecs / 60;

  std::stringstream uptime;

  uptime << COLOR << "uptime:\t\033[0m";

  if (day)
    uptime << day << "d ";
  if (hour)
    uptime << hour << "h ";

  uptime << minutes << "m\n";

  return uptime.str();
}

const std::string OSName() {
  std::string line;
  std::ifstream infile;
  constexpr std::string_view prettyName{"PRETTY_NAME=\""};
  infile.open("/etc/os-release");

  if (infile.bad()) {
    perror("unable to open /etc/os-release");
    exit(EXIT_FAILURE);
  }

  while (infile.good()) {
    std::getline(infile, line);
    size_t pos = line.find(prettyName);
    if(pos!=std::string::npos)
      break;
  }

  line.erase(line.begin(), line.begin() + prettyName.length());
  line.erase(line.end() - 1);
  std::stringstream name;
  name << COLOR << "os:\033[0m\t" << line << "\n";

  return name.str();
}

const std::string Host() {
  std::string productName;
  std::string productFamily;
  std::ifstream infile;
  infile.open("/sys/devices/virtual/dmi/id/product_name");
  if (infile.good()) {
    std::getline(infile, productName);
    if (productName.find("OEM")!=std::string::npos ||
        productName.find("O.E.M.")!=std::string::npos ||
	productName.find("Default")!=std::string::npos ||
	productName.find("INVALID")!=std::string::npos ||
	productName.find("Not")!=std::string::npos ||
	productName.find("System")!=std::string::npos) {
      infile.open("/sys/devices/virtual/dmi/id/board_name");
      std::getline(infile, productName);
    }
  }
  infile.open("/sys/devices/virtual/dmi/id/product_family");
  if (infile.good()) {
    std::getline(infile, productFamily);
    if (productFamily.find("OEM")!=std::string::npos ||
	productFamily.find("O.E.M.")!=std::string::npos ||
	productFamily.find("Default")!=std::string::npos ||
	productFamily.find("INVALID")!=std::string::npos ||
	productFamily.find("Not")!=std::string::npos ||
	productFamily.find("System")!=std::string::npos ||
	productFamily == productName)
      productFamily.clear();
  }
  std::stringstream host;
  host << COLOR << "host:\t\033[0m" << productName << ' ' << productFamily << '\n';

  return host.str();
}

std::string shellCmd(const char *icmd) {
  FILE *stream;
  const int maxBuffer = 256;
  char buffer[maxBuffer];
  std::string cmdo;
  stream = popen(icmd, "r");

  if (stream) {
    while (!feof(stream))
      if (fgets(buffer, maxBuffer, stream) != NULL) cmdo.append(buffer);
        pclose(stream);
  }
  return cmdo;
}

uint Pacman(std::string path) {
  std::filesystem::path pkgfolder = path;
  using std::filesystem::directory_iterator;
  return std::distance(directory_iterator(pkgfolder), directory_iterator{});
}

uint Portage(std::string path) {
  std::filesystem::path pkgfolder = path;
  uint totalSubdirs = 0;
  using std::filesystem::recursive_directory_iterator;
  for(auto i = recursive_directory_iterator(path) ; i!= recursive_directory_iterator() ; ++i) {
    if(i.depth() == 1) {
    i.disable_recursion_pending();
    totalSubdirs++;
    }
  }
  return totalSubdirs;
}

const std::string Packages() {
  std::string pkg;
  std::string pkgmgr;

  if (std::filesystem::exists("/etc/portage")) {
    pkg.append(std::to_string(Portage("/var/db/pkg")));
    pkg.append(" (emerge) ");
  }
  if (std::filesystem::exists("/etc/pacman.d")) {
    pkg.append(std::to_string(Pacman("/var/lib/pacman/local/")-1));
    pkg.append(" (pacman) ");
  }
  if (std::filesystem::exists("/etc/apt")) {
    pkgmgr = shellCmd("dpkg --get-selections | wc -l 2>&1");
    pkg.append(pkgmgr.begin(), pkgmgr.end()-1);
    pkg.append(" (dpkg) ");
  }
  if (std::filesystem::exists("/nix")) {
    if (std::filesystem::exists("/etc/nix")) {
      pkgmgr = shellCmd("nix-store --query --requisites /run/current-system | wc -l");
    } else {
      pkgmgr = shellCmd("nix-env -q | wc -l");
    }
    pkg.append(pkgmgr.begin(), pkgmgr.end()-1);
    pkg.append(" (nix) ");
  }

  std::stringstream pkgs;
  pkgs << COLOR << "pkgs:\t\033[0m" << pkg << "\n";
	
  return pkgs.str();
}

const std::string Mem() {
  unsigned long memtotal = sysinfoLocal.totalram / 1024;
  unsigned long memavail;
  constexpr std::string_view memAvailable{"MemAvailable:"};
  std::string memavailstr;
  std::string searchtoken;
  std::ifstream infile("/proc/meminfo");
  if (infile.bad()) {
    perror("unable to open /proc/meminfo");
    exit(EXIT_FAILURE);
  }
  while (infile.good()) {
    std::getline(infile, searchtoken);
    size_t mpos = searchtoken.find(memAvailable);
    if(mpos!=std::string::npos) {
      memavailstr = searchtoken;
      break;
    }
  }
  memavailstr.erase(memavailstr.begin(), memavailstr.begin() + memAvailable.length());
  std::istringstream mema (memavailstr);
  mema >> memavail;

  unsigned long memused = memtotal - memavail;
  memused /= 1024;
  memtotal /= 1024;
  std::stringstream mem;
  mem << COLOR << "memory:\t\033[0m" << memused << "M / " << memtotal << "M\n";

  return mem.str();
}

const std::string User() {
  std::stringstream user;
  user << HOSTCOLOR << getlogin() << "@" << unameLocal.nodename << "\033[0m\n";
  return user.str();
}

const std::string Kernel() {
  std::stringstream kernel;
  kernel << COLOR << "kernel:\t\033[0m" << unameLocal.release << "\033[0m\n";
  return kernel.str();
}

int main() {
  if(uname(&unameLocal) != 0) {
    perror("uname error");
    exit(EXIT_FAILURE);
  }
  if (sysinfo(&sysinfoLocal) != 0) {
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
