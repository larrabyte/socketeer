# ------------------------------------------------------
# Makefile for socketeer, made by the larrabyte himself.
# ------------------------------------------------------
SRCFILES := $(wildcard src/*.cpp)
OBJFILES := $(SRCFILES:src/%.cpp=obj/%.o)
INCLUDES := -Iinclude -I/opt/homebrew/include
WARNINGS := -Wall -Wextra -Wpedantic
CFLAGS   := $(WARNINGS) $(INCLUDES) -std=c++11
LFLAGS   := -lpthread

ifdef CROSS
CFLAGS += -fPIE --target=x86_64-pc-linux-gnu --sysroot=/opt/sysroots/x86_64-linux
LFLAGS += -pie -fuse-ld=/opt/homebrew/bin/x86_64-elf-ld
endif

all: $(OBJFILES)
	@$(CXX) $(CFLAGS) $(OBJFILES) -o bin/socketeer $(LFLAGS)
	@printf "[linking] binary created.\n"

clean:
	@rm -f obj/*
	@printf "[cleaner] removed object files.\n"
	@rm -f bin/*
	@printf "[cleaner] removed built binaries.\n"

obj/%.o: src/%.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
	@printf "[cpp2obj] $< compiled.\n"
