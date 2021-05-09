CC=clang++
CFLAGS=-g -Wall
TARGET=yafetch

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)

install:
	@install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
