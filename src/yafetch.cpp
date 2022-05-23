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
#include "yafetch.h"

#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

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
struct Colors colors;

std::string Uptime() {
  time_t totalSecs{reinterpret_cast<time_t>(Sysinfo.uptime)};
  auto formattedTime = gmtime(&totalSecs);
  std::string uptime;

  auto hours = formattedTime->tm_hour;
  auto minutes = formattedTime->tm_min;
  auto seconds = formattedTime->tm_sec;

  if (hours)
    uptime += std::to_string(hours) + "h ";
  if (minutes)
    uptime += std::to_string(minutes) + "m ";
  if (seconds)
    uptime += std::to_string(seconds) + "s";

  return uptime;
}

std::string OSName() {
  auto stream = std::ifstream("/etc/os-release");
  std::string prettyName;

  for (std::string line; std::getline(stream, line);) {
    if (line.find("PRETTY_NAME") != std::string::npos)
      prettyName = line.substr(line.find("=") + 2, (line.length() - line.find("=") - 3));
  }

  return prettyName;
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

  host << productName << ' ' << productFamily;
  return host.str();
}

std::string Mem() {
  constexpr std::string_view searchPattern{"MemAvailable:"};
  unsigned long memTotal{Sysinfo.totalram / 1024};
  unsigned long memAvail;
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
  memory << memUsed << "M / " << memTotal << "M";

  return memory.str();
}

std::string User() { return colors.wraphost(std::string(getlogin()) + "@" + Uname.nodename); }

std::string Kernel() { return (static_cast<std::string>(Uname.release) + "\033[0m"); }

int main() {
  if (uname(&Uname) != 0)
    throw std::runtime_error("Unable to access utsname.h");
  if (sysinfo(&Sysinfo) != 0)
    throw std::runtime_error("Unable to access sysinfo.h");

  std::string line;
  std::istringstream logo;
  std::string osName;
  std::string out;

  osName = OSName();
  PackageManagers *pkg{nullptr};

  if (osName.find("NixOS") != std::string::npos) {
    logo.str("\033[1;34m  \\\\  \033[1;36m\\\\ //\t\n\
\033[1;34m ==\\\\__\033[1;36m\\\\/ \033[1;34m//\t\n\
\033[1;36m   //   \033[1;36m\\\\\033[1;34m//\t\n\
\033[1;36m==//     \033[1;34m//==\t\n\
\033[1;36m //\033[1;34m\\\\\033[1;36m___\033[1;34m//\t\n\
\033[1;36m// \033[1;34m/\\\\  \033[1;36m\\\\==\t\n\
\033[1;34m  // \\\\  \033[1;36m\\\\\t");
    pkg = new Nix();
  } else if (osName.find("Arch") != std::string::npos) {
    logo.str("\033[1;36m       /\\\t\n\
\033[1;36m      /  \\\t\n\
\033[1;36m     /\\   \\\t\n\
\033[1;34m    /      \\\t\n\
\033[1;34m   /   ,,   \\\t\n\
\033[1;34m  /   |  |  -\\\t\n\
\033[1;34m /_-''    ''-_\\\t");
    pkg = new Pacman();
  } else if (osName.find("Gentoo") != std::string::npos) {
    logo.str("\033[1;35m _-----_\t\n\
\033[1;35m(       \\\t\n\
\033[1;35m\\    0   \\\t\n\
\033[1;0m \\        )\t\n\
\033[1;0m /      _/\t\n\
\033[1;0m(     _-\t\n\
\033[1;0m\\____-\t\t");
    pkg = new Portage();
  } else {
    pkg = new PackageManagers();
    logo.str("\033[1;34m    ___\t\t\n\
\033[1;34m   (\033[1;0m..\033[1;34m |\t\n\
\033[1;34m   (\033[1;35m<>\033[1;34m |\t\n\
\033[1;34m  / \033[1;0m__\033[1;34m  \\\t\n\
\033[1;34m ( \033[1;0m/  \\\033[1;34m /|\t\n\
\033[1;35m_\033[1;34m/\\ \033[1;0m__)\033[1;34m/\033[1;35m_\033[1;34m)\t\n\
\033[1;35m\\/\033[1;34m-____\033[1;35m\\/\t\n");
  }

  std::array<std::string, 7> info = {
      User(),
      colors.wrap("os:\t") + osName,
      colors.wrap("host:\t") + Host(),
      colors.wrap("kernel:\t") + Kernel(),
      colors.wrap("pkgs:\t") + pkg->Packages(),
      colors.wrap("uptime:\t") + Uptime(),
      colors.wrap("memory:\t") + Mem(),
  };

  if (pkg)
    delete pkg;

  for (const auto &s : info) {
    std::getline(logo, line);
    out += line + s + '\n';
  }
  std::cout << out;
}
