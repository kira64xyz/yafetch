# Copyright (C) 2021 Ashley Chiara <ashley@kira64.xyz>
# This file is part of yafetch.

# yafetch is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3, or (at your option) any later
# version.

# yafetch is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.

# You should have received a copy of the GNU General Public License
# along with yafetch; see the file COPYING.  If not see
# <http://www.gnu.org/licenses/>.

CFLAGS=-Wall -std=c++17 -g -O3
TARGET=yafetch

all: $(TARGET)

$(TARGET): $(TARGET).cpp config.h
	$(CXX) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	rm -f $(TARGET)

install: $(TARGET)
	@install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
