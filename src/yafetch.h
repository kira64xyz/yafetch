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
#include <memory>
#include <string>

std::string OSName(), Packages(), Uptime(), Mem(), Host(), Kernel(), User();

inline std::string shellCmd(const char *input) {
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

class PackageManagers {
public:
  virtual std::string Packages() { return "unknown"; };
  virtual ~PackageManagers() = default;
};

class Nix : public PackageManagers {
public:
  virtual std::string Packages() { return shellCmd("nix-store --query --requisites /run/current-system | wc -l"); };

  virtual ~Nix() = default;
};

class Pacman : public PackageManagers {
public:
  virtual std::string Packages() {
    std::filesystem::path pkgFolder{"/var/lib/pacman/local/"};
    using std::filesystem::directory_iterator;
    return std::to_string(std::distance(directory_iterator(pkgFolder), directory_iterator{}));
  };

  virtual ~Pacman() = default;
};

class Portage : public PackageManagers {
public:
  virtual std::string Packages() {
    std::string path{"/var/db/pkg/"};
    std::filesystem::path pkgFolder{path};
    unsigned int totalSubdirs{0};

    using std::filesystem::recursive_directory_iterator;
    for (auto i{recursive_directory_iterator(path)}; i != recursive_directory_iterator(); ++i) {
      if (i.depth() == 1) {
        i.disable_recursion_pending();
        totalSubdirs++;
      }
    }

    return std::to_string(totalSubdirs);
  };

  virtual ~Portage() = default;
};
