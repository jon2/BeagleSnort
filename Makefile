# Generic Makefile
# Set the "SOURCES" and the "EXECUTABLE" variables to whatever is needed.  Recycle often.

CC=arm-linux-gnueabihf-gcc
CFLAGS=-g -Wall -c -I. -pthread
LDFLAGS=-pthread
SOURCES=gpio-utils.c ledctrl.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=beaglesnort

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o beaglesnort *~

