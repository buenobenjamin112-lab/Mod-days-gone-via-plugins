
TARGET = DaysGoneSpawner.prx

OO_PS4_TOOLCHAIN ?= /opt/openorbis
GOLDHEN_SDK ?= /opt/goldhen-sdk

CC = clang
CXX = clang++
LD = ld.lld

SDK_SOURCE = $(GOLDHEN_SDK)/source
SDK_INCLUDE = $(GOLDHEN_SDK)/include

CFLAGS = --target=x86_64-pc-freebsd12-elf \
         -fPIC -funwind-tables -c \
         -isysroot $(OO_PS4_TOOLCHAIN) \
         -isystem $(OO_PS4_TOOLCHAIN)/include \
         -I$(SDK_INCLUDE)

CXXFLAGS = $(CFLAGS) \
           -isystem $(OO_PS4_TOOLCHAIN)/include/c++/v1

LDFLAGS = -m elf_x86_64 -pie \
          --script $(OO_PS4_TOOLCHAIN)/link.x \
          -e _init \
          --eh-frame-hdr \
          -L$(OO_PS4_TOOLCHAIN)/lib \
          -lSceLibcInternal -lkernel -lSceSysmodule

SDK_SRCS = $(wildcard $(SDK_SOURCE)/*.c)
SDK_OBJS = $(patsubst $(SDK_SOURCE)/%.c,build/sdk_%.o,$(SDK_SRCS))

CRT = build/crtprx.o
MAIN = build/main.o

all: $(TARGET)

build:
	mkdir -p build

build/main.o: src/main.cpp | build
	$(CXX) $(CXXFLAGS) -o $@ $<

build/sdk_%.o: $(SDK_SOURCE)/%.c | build
	$(CC) $(CFLAGS) -o $@ $<

$(CRT): $(GOLDHEN_SDK)/crt/crtprx.c | build
	$(CC) -target x86_64-pc-linux-gnu \
	      -ffreestanding -nostdlib -fno-builtin -fPIC \
	      -isysroot $(OO_PS4_TOOLCHAIN) \
	      -isystem $(OO_PS4_TOOLCHAIN)/include \
	      -c $< -o $@

$(TARGET): $(MAIN) $(SDK_OBJS) $(CRT)
	$(LD) $(MAIN) $(SDK_OBJS) $(CRT) \
	      -o build/DaysGoneSpawner.elf $(LDFLAGS)
	$(OO_PS4_TOOLCHAIN)/bin/linux/create-fself \
	      -in=build/DaysGoneSpawner.elf \
	      -out=$(TARGET) \
	      --lib=$(TARGET) \
	      --paid 0x3800000000000011

clean:
	rm -rf build $(TARGET)

.PHONY: all clean
