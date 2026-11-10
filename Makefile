TARGET = DaysGoneSpawner
SRCS = src/main.cpp src/cheats.cpp
OBJS = $(SRCS:.cpp=.o)

CC = clang
CXX = clang++
LD = ld.lld

OO_PS4_TOOLCHAIN ?= /opt/openorbis

CFLAGS = -O3 -Wall -fPIC -target x86_64-unknown-freebsd-elf -isystem $(OO_PS4_TOOLCHAIN)/include
CXXFLAGS = $(CFLAGS) -std=c++11
LDFLAGS = -shared -L$(OO_PS4_TOOLCHAIN)/lib

all: $(TARGET).prx

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).prx: $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET).elf $(OBJS)
	orbis-ld -shared -o $(TARGET).prx $(OBJS)

clean:
	rm -f src/*.o $(TARGET).elf $(TARGET).prx
