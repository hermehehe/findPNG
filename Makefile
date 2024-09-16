CC=gcc
CFLAGS=-Wall -Wextra -O2 -g -std=c99
LD=gcc
LDFLAGS=-g
LDLIBS = -lz

# source files
FINDPNG_SRCS   = findpng.c crc.c

# object files
FINDPNG_OBJS = $(FINDPNG_SRCS:.c=.o)

# targets
TARGETS= findpng

# header files
# DEPS = png_def.h

all: $(TARGETS)

findpng: $(FINDPNG_OBJS) 
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<

%.d: %.c
	gcc -MM -MF $@ $<  

.PHONY: clean
clean:
	rm -f *.d *.o $(FINDPNG_OBJS) $(TARGETS)
