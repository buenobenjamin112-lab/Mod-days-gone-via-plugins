TARGET = DaysGoneSpawner
OBJS = src/main.o src/cheats.o

CC = clang
CXX = clang++
LD = ld.lld

CFLAGS = -O3 -Wall -fPIC -isystem $(OO_PS4_TOOLCHAIN)/include
CXXFLAGS = $(CFLAGS) -std=c++11
LDFLAGS = -shared -L$(OO_PS4_TOOLCHAIN)/lib

all: $(TARGET).prx

$(TARGET).prx: $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET).elf $(OBJS)
	orbis-ld -shared -o $(TARGET).prx $(OBJS)

clean:
	rm -f src/*.o $(TARGET).elf $(TARGET).prx
