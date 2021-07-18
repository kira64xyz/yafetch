#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "config.h"

#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include <cstdint>
using u128 = __uint128_t; //!< Unsigned 128-bit integer
using u64 = __uint64_t;   //!< Unsigned 64-bit integer
using u32 = __uint32_t;   //!< Unsigned 32-bit integer
using u16 = __uint16_t;   //!< Unsigned 16-bit integer
using u8 = __uint8_t;     //!< Unsigned 8-bit integer
using i128 = __int128_t;  //!< Signed 128-bit integer
using i64 = __int64_t;    //!< Signed 64-bit integer
using i32 = __int32_t;    //!< Signed 32-bit integer
using i16 = __int16_t;    //!< Signed 16-bit integer
using i8 = __int8_t;      //!< Signed 8-bit integer

struct sysinfo Sysinfo;
struct utsname Uname;

std::string Uptime() {
  long totalSecs{Sysinfo.uptime};
  constexpr u8 SecondsInMinute{60};
  constexpr u16 SecondsInHour{SecondsInMinute * 60};
  constexpr u32 SecondsInDay{SecondsInHour * 24};

  u64 day{static_cast<u64>(totalSecs / SecondsInDay)};
  totalSecs %= SecondsInDay;
  u16 hour{static_cast<u8>(totalSecs / SecondsInHour)};
  totalSecs %= SecondsInHour;
  u16 minutes{static_cast<u8>(totalSecs / SecondsInMinute)};

  std::stringstream uptime;

  uptime << COLOR << "uptime:\t\033[0m";

  if (day)
    uptime << day << "d ";
  if (hour)
    uptime << hour << "h ";

  uptime << minutes << "m\n";

  return uptime.str();
}

std::string OSName() {
  std::string line;
  std::ifstream infile;
  constexpr std::string_view prettyName{"PRETTY_NAME=\""};
  infile.open("/etc/os-release");

  while (infile.good()) {
    std::getline(infile, line);
    size_t pos{line.find(prettyName)};
    if (pos != std::string::npos)
      break;
  }

  line = line.substr(prettyName.length(), line.length() - (prettyName.length() + 1));
  std::stringstream name;
  name << COLOR << "os:\033[0m\t" << line << "\n";

  return name.str();
}

std::string Host() {
  std::string productName;
  std::string productFamily;
  std::ifstream infile;

  auto productCheck{[](const std::string &name) {
    // clang-format off
    if (name.find("OEM")     != std::string::npos ||
	name.find("O.E.M.")  != std::string::npos ||
        name.find("Default") != std::string::npos ||
	name.find("INVALID") != std::string::npos ||
        name.find("Not")     != std::string::npos ||
	name.find("System")  != std::string::npos)
      return true;
    return false;
    // clang-format on
  }};

  infile.open("/sys/devices/virtual/dmi/id/product_name");
  if (infile.good()) {
    std::getline(infile, productName);
    if (productCheck(productName)) {
      infile.open("/sys/devices/virtual/dmi/id/board_name");
      std::getline(infile, productName);
    }
  }
  infile.open("/sys/devices/virtual/dmi/id/product_family");
  if (infile.good()) {
    std::getline(infile, productFamily);
    if (productCheck(productFamily) || productFamily == productName)
      productFamily.clear();
  }
  std::stringstream host;
  host << COLOR << "host:\t\033[0m" << productName << ' ' << productFamily << '\n';

  return host.str();
}

std::string shellCmd(const char *input) {
  std::unique_ptr<FILE, decltype(&pclose)> stream{popen(input, "r"), &pclose};

  std::string output;
  if (stream) {
    while (!feof(stream.get())) {
      auto offset{output.size()};
      output.resize(output.size() + 256);
      if (fgets(output.data() + offset, output.size() - offset, stream.get()) == NULL)
        break;
      if (ferror(stream.get())) {
        output.resize(offset);
        break;
      }
      output.resize(std::distance(output.begin(), std::find(output.begin() + offset, output.end(), '\0') - 1));
    }
  }
  if (output.back() == '\n')
    output.pop_back();
  return output;
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
  for (auto i{recursive_directory_iterator(path)}; i != recursive_directory_iterator(); ++i) {
    if (i.depth() == 1) {
      i.disable_recursion_pending();
      totalSubdirs++;
    }
  }
  return totalSubdirs;
}

std::string Packages() {
  std::stringstream pkg;

  if (std::filesystem::exists("/etc/portage")) {
    pkg << std::to_string(Portage("/var/db/pkg")) << " (emerge) ";
  }
  if (std::filesystem::exists("/etc/pacman.d")) {
    pkg << std::to_string(Pacman("/var/lib/pacman/local/") - 1) << " (pacman) ";
  }
  if (std::filesystem::exists("/etc/apt")) {
    pkg << shellCmd("dpkg --get-selections | wc -l 2>&1") << " (dpkg) ";
  }
  if (std::filesystem::exists("/nix")) {
    if (std::filesystem::exists("/etc/nix")) {
      pkg << shellCmd("nix-store --query --requisites /run/current-system | wc -l");
    } else {
      pkg << shellCmd("nix-env -q | wc -l");
    }
    pkg << (" (nix) ");
  }

  std::stringstream pkgs;
  pkgs << COLOR << "pkgs:\t\033[0m" << pkg.str() << '\n';

  return pkgs.str();
}

std::string Mem() {
  unsigned long memTotal{Sysinfo.totalram / 1024};
  unsigned long memAvail;
  constexpr std::string_view memAvailable{"MemAvailable:"};
  std::string memAvailStr;
  std::string searchToken;
  std::ifstream infile("/proc/meminfo");
  while (infile.good()) {
    std::getline(infile, searchToken);
    size_t mpos = searchToken.find(memAvailable);
    if (mpos != std::string::npos) {
      memAvailStr = searchToken;
      break;
    }
  }
  memAvailStr.erase(memAvailStr.begin(), memAvailStr.begin() + memAvailable.length());
  std::istringstream mema(memAvailStr);
  mema >> memAvail;

  unsigned long memUsed{memTotal - memAvail};
  memUsed /= 1024;
  memTotal /= 1024;
  std::stringstream mem;
  mem << COLOR << "memory:\t\033[0m" << memUsed << "M / " << memTotal << "M\n";

  return mem.str();
}

std::string User() {
  std::stringstream user;
  user << HOSTCOLOR << getlogin() << "@" << Uname.nodename << "\033[0m\n";
  return user.str();
}

std::string Kernel() {
  std::stringstream kernel;
  kernel << COLOR << "kernel:\t\033[0m" << Uname.release << "\033[0m\n";
  return kernel.str();
}

int main() {
  if (uname(&Uname) != 0)
    throw std::runtime_error("Unable to access utsname.h");
  if (sysinfo(&Sysinfo) != 0)
    throw std::runtime_error("Unable to access sysinfo.h");

  std::string line;
  std::istringstream f(logo);
  std::stringstream inf;

  for (const auto &function : info) {
    std::getline(f, line);
    inf << line << function();
  }
  std::cout << inf.str();
}
