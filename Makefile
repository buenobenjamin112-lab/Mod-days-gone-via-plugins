TARGET = DaysGoneSpawner
SRCS = src/main.cpp src/cheats.cpp
OBJS = $(SRCS:.cpp=.o)

CC = clang
CXX = clang++
LD = ld.lld

# Define o caminho do toolchain do OpenOrbis no ambiente do GitHub Actions
OO_PS4_TOOLCHAIN ?= /opt/openorbis

# Adiciona as flags de include apontando diretamente para as pastas de desenvolvimento do PS4
CFLAGS = -O3 -Wall -fPIC -target x86_64-unknown-freebsd-elf \
         -I$(OO_PS4_TOOLCHAIN)/include \
         -I$(OO_PS4_TOOLCHAIN)/include/libcxx \
         -I$(OO_PS4_TOOLCHAIN)/include/orbis

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
