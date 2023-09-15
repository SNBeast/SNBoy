SRCDIR = source
SOURCES = $(wildcard $(SRCDIR)/*.c)
INCDIR = include
INCLUDE = $(wildcard $(INCDIR)/*.h)
OBJDIR = object
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
TARGET = snboy.exe

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c2x -O3 -flto $(shell sdl2-config --cflags)
LDLIBS = $(shell sdl2-config --libs)

all: $(OBJDIR) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ $(LDLIBS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDE)
	$(CC) -c $(CFLAGS) -I$(INCDIR) $< -o $@

$(OBJDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(TARGET)
