CFLAGS=-g -Wall -std=c++17 -O3
TARGET=yafetch

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)

install:
	@install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
