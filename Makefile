CC=g++
CFLAGS=-g -Wall
TARGET=afetch

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)

install:
	@install -Dm755 afetch $(DESTDIR)$(PREFIX)/bin/afetch

uninstall:
	@rm -f $(DESTDIR)$(PREFIX)/bin/afetch
