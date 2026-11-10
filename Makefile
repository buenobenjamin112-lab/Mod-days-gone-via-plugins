
TARGET = DaysGoneSpawner.prx

OO_PS4_TOOLCHAIN ?= /opt/openorbis
GOLDHEN_SDK ?= /opt/goldhen-sdk

CC = clang
CXX = clang++
LD = ld.lld

CFLAGS = --target=x86_64-pc-freebsd12-elf \
         -fPIC -funwind-tables -c \
         -isysroot $(OO_PS4_TOOLCHAIN) \
         -isystem $(OO_PS4_TOOLCHAIN)/include \
         -I$(GOLDHEN_SDK)/include

CXXFLAGS = $(CFLAGS) \
           -isystem $(OO_PS4_TOOLCHAIN)/include/c++/v1

LDFLAGS = -m elf_x86_64 -pie \
          --script $(OO_PS4_TOOLCHAIN)/link.x \
          --eh-frame-hdr \
          -L$(OO_PS4_TOOLCHAIN)/lib \
          -lkernel -lc -lc++

SDK_SRCS = $(wildcard $(GOLDHEN_SDK)/source/*.c)

all: $(TARGET)

build:
	mkdir -p build

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -o $@ $<

build/sdk_%.o: $(GOLDHEN_SDK)/source/%.c | build
	$(CC) $(CFLAGS) -I$(GOLDHEN_SDK)/include -o $@ $<

build/main.o: src/main.cpp | build
	$(CXX) $(CXXFLAGS) -o $@ $<

build/crtprx.o: $(OO_PS4_TOOLCHAIN)/samples/_common/crtprx.c | build
	$(CC) --target=x86_64-pc-freebsd12-elf \
	      -fPIC -funwind-tables -c \
	      -isysroot $(OO_PS4_TOOLCHAIN) \
	      -isystem $(OO_PS4_TOOLCHAIN)/include \
	      -o $@ $<

build/sdk_%.o: $(GOLDHEN_SDK)/source/%.c | build
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET): build/main.o $(patsubst $(GOLDHEN_SDK)/source/%.c,build/sdk_%.o,$(SDK_SRCS)) build/crtprx.o
	$(LD) $^ -o build/DaysGoneSpawner.elf $(LDFLAGS)
	$(OO_PS4_TOOLCHAIN)/bin/linux/create-fself \
	    -in=build/DaysGoneSpawner.elf \
	    -out=$(TARGET) --lib=$(TARGET) --paid 0x3800000000000011

clean:
	rm -rf build $(TARGET)

.PHONY: all clean build
