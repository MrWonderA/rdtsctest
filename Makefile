CC ?= gcc
AR ?= ar
CFLAGS ?= -Wall -Wextra -O2
LDFLAGS ?=

TARGET_LIB = librdtsctest.a
TARGET_SO = librdtsctest.so

SOURCES = rdtsc_test.c
OBJECTS = $(SOURCES:.c=.o)

UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
    ARCH_FLAGS = -march=native
else ifeq ($(UNAME_M),i386)
    ARCH_FLAGS = -march=native
else ifeq ($(UNAME_M),i686)
    ARCH_FLAGS = -march=native
else ifeq ($(UNAME_M),aarch64)
    ARCH_FLAGS = -march=armv8-a
else ifeq ($(UNAME_M),armv7l)
    ARCH_FLAGS = -march=armv7-a -mfpu=neon
else ifeq ($(UNAME_M),arm)
    ARCH_FLAGS = -march=armv7-a
endif

all: $(TARGET_LIB) $(TARGET_SO)

$(TARGET_LIB): $(OBJECTS)
	$(AR) rcs $@ $^

$(TARGET_SO): $(OBJECTS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

%.o: %.c rdtsc_test.h
	$(CC) $(CFLAGS) $(ARCH_FLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET_LIB) $(TARGET_SO)

.PHONY: all clean
