CC = clang

CPPFLAGS += -DPOSIX -DHAVE_GETOPT -DHARD_COLORS -DUSE_ZLIB
CFLAGS ?= -O2
CFLAGS += -std=gnu89 -MMD -MP
LDLIBS += -lz -ltermcap

JZIP_OBJS = \
	jzip.o \
	control.o \
	extern.o \
	fileio.o \
	input.o \
	interpre.o \
	license.o \
	math.o \
	memory.o \
	object.o \
	operand.o \
	osdepend.o \
	property.o \
	quetzal.o \
	screen.o \
	text.o \
	variable.o \
	unixio.o

TARGETS = jzip ckifzs
DEPS = $(JZIP_OBJS:.o=.d) ckifzs.d

.PHONY: all clean

all: $(TARGETS)

jzip: $(JZIP_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(JZIP_OBJS) $(LDLIBS)

ckifzs: ckifzs.o
	$(CC) $(LDFLAGS) -o $@ ckifzs.o

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGETS) $(JZIP_OBJS) ckifzs.o $(DEPS)

-include $(DEPS)
