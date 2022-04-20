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
using u64 = uint_fast64_t; //!< Unsigned 64-bit integer
using u32 = uint_fast32_t; //!< Unsigned 32-bit integer
using u16 = uint_fast16_t; //!< Unsigned 16-bit integer
using u8 = uint_fast8_t;   //!< Unsigned 8-bit integer
using i64 = int_fast64_t;  //!< Signed 64-bit integer
using i32 = int_fast32_t;  //!< Signed 32-bit integer
using i16 = int_fast16_t;  //!< Signed 16-bit integer
using i8 = int_fast8_t;    //!< Signed 8-bit integer

struct sysinfo Sysinfo;
struct utsname Uname;

std::string Uptime() {
  unsigned long totalSecs{static_cast<unsigned long>(Sysinfo.uptime)};
  std::ostringstream uptime;
  constexpr u8 SecondsInMinute{60};
  constexpr u16 SecondsInHour{SecondsInMinute * 60};
  constexpr u32 SecondsInDay{SecondsInHour * 24};

  u64 day{static_cast<u64>(totalSecs / SecondsInDay)};
  totalSecs %= SecondsInDay;
  u16 hour{static_cast<u8>(totalSecs / SecondsInHour)};
  totalSecs %= SecondsInHour;
  u16 minutes{static_cast<u8>(totalSecs / SecondsInMinute)};

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
  std::ostringstream name;
  constexpr std::string_view prettyName{"PRETTY_NAME=\""};
  infile.open("/etc/os-release");

  while (infile.good()) {
    std::getline(infile, line);
    size_t pos{line.find(prettyName)};
    if (pos != std::string::npos)
      break;
  }

  name << COLOR << "os:\033[0m\t" << line.substr(prettyName.length(), line.length() - (prettyName.length() + 1)) << "\n";

  return name.str();
}

std::string Host() {
  std::string productName;
  std::string productFamily;
  std::ostringstream host;
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
  infile.close();
  infile.open("/sys/devices/virtual/dmi/id/product_family");
  if (infile.good()) {
    std::getline(infile, productFamily);
    if (productCheck(productFamily) || productFamily == productName)
      productFamily.clear();
  }
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

unsigned int Pacman(std::string path) {
  std::filesystem::path pkgFolder{path};
  using std::filesystem::directory_iterator;
  return std::distance(directory_iterator(pkgFolder), directory_iterator{});
}

unsigned int Portage(std::string path) {
  std::filesystem::path pkgFolder{path};
  unsigned int totalSubdirs{0};
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
  std::ostringstream pkg;
  std::ostringstream packageOutput;

  if (std::filesystem::exists("/etc/apt"))
    pkg << shellCmd("dpkg --get-selections | wc -l 2>&1") << " (dpkg) ";

  if (std::filesystem::exists("/etc/portage"))
    pkg << std::to_string(Portage("/var/db/pkg")) << " (emerge) ";

  if (std::filesystem::exists("/nix")) {
    if (std::filesystem::exists("/etc/nix")) {
      pkg << shellCmd("nix-store --query --requisites /run/current-system | wc -l");
    } else {
      pkg << shellCmd("nix-env -q | wc -l");
    }
    pkg << (" (nix) ");
  }

  if (std::filesystem::exists("/etc/pacman.d"))
    pkg << std::to_string(Pacman("/var/lib/pacman/local/") - 1) << " (pacman) ";

  if (std::filesystem::exists("/etc/xbps.d"))
    pkg << shellCmd("xbps-query -l | wc -l") << " (xbps) ";

  packageOutput << COLOR << "pkgs:\t\033[0m" << pkg.str() << '\n';

  return packageOutput.str();
}

std::string Mem() {
  unsigned long memTotal{Sysinfo.totalram / 1024};
  unsigned long memAvail;
  constexpr std::string_view searchPattern{"MemAvailable:"};
  std::string memAvailStr;
  std::string searchToken;
  std::ostringstream memory;
  std::ifstream infile("/proc/meminfo");
  while (infile.good()) {
    std::getline(infile, searchToken);
    size_t mpos{searchToken.find(searchPattern)};
    if (mpos != std::string::npos) {
      memAvailStr = searchToken;
      break;
    }
  }
  memAvailStr.erase(memAvailStr.begin(), memAvailStr.begin() + searchPattern.length());
  memAvail = std::stol(memAvailStr);

  unsigned long memUsed{memTotal - memAvail};
  memUsed /= 1024;
  memTotal /= 1024;
  memory << COLOR << "memory:\t\033[0m" << memUsed << "M / " << memTotal << "M\n";

  return memory.str();
}

std::string User() {
  std::ostringstream user;
  user << HOSTCOLOR << getlogin() << "@" << Uname.nodename << "\033[0m\n";
  return user.str();
}

std::string Kernel() {
  std::ostringstream kernel;
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
  std::ostringstream inf;

  for (const auto &function : info) {
    std::getline(f, line);
    inf << line << function();
  }
  std::cout << inf.str();
}
