# ----------------------------------------------------
# Makefile for socketeer, made by the larrabyte himself.
# ----------------------------------------------------
.PHONY: default clean win 

OS ?= linus

# ----------------------------------
# Assember, compiler and QEMU flags.
# ----------------------------------

ifeq ($(OS), linus)
	CC := x86_64-pc-linux-gnu-gcc
else
	CC := x86_64-w64-mingw32-gcc
endif

WINFLAGS := -lws2_32
WARNINGS := -Wall -Wextra -Wpedantic
CFLAGS   := $(WARNINGS) -O3 -flto -march=native

# -----------------------------
# Required directories & files.
# -----------------------------
HDRDIR := src/head
SRCDIR := src
OBJDIR := obj

SRCFILES := $(wildcard $(SRCDIR)/*.c)
OBJFILES := $(SRCFILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# --------
# Targets.
# --------
default: $(OS)

clean:
	@rm -f $(OBJDIR)/*.o
	@printf "[wipe] Deleted object files from the object folder.\n"
	@rm -f bin/socketeer && rm -f bin/socketeer.exe
	@printf "[wipe] Deleted built executables.\n"

linus: $(OBJFILES)
	@printf "[link] Linking object files and creating executable.\n"
	@$(CC) $(CFLAGS) $(OBJFILES) -o bin/socketeer

win: $(OBJFILES)
	@printf "[link] Linking object files and creating Windows executable.\n"
	@$(CC) $(CFLAGS) $(OBJFILES) -o bin/socketeer.exe $(WINFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@printf "[gccc] $< compiled.\n"
	@$(CC) $(CFLAGS) -c $< -o $@
