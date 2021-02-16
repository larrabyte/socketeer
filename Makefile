# ------------------------------------------------------
# Makefile for socketeer, made by the larrabyte himself.
# ------------------------------------------------------
WARNINGS := -Wall -Wextra -Wpedantic
INCLUDES := -Iinclude -I/opt/homebrew/Cellar/asio/1.18.1/include
CFLAGS   := $(WARNINGS) $(INCLUDES) -std=c++11
CC	     := c++

SRCDIR   := src
OBJDIR   := obj
SRCFILES := $(wildcard $(SRCDIR)/*.cpp)
OBJFILES := $(SRCFILES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(OBJFILES)
	@c++ $(CFLAGS) $(OBJFILES) -o bin/socketeer -lpthread
	@printf "[linking] binary created.\n"

clean:
	@rm -f $(OBJDIR)/*
	@printf "[cleaner] removed object files.\n"
	@rm -f bin/*
	@printf "[cleaner] removed built binaries.\n"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
	@printf "[cpp2obj] $< compiled.\n"
