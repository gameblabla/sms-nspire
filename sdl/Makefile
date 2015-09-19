
# Makefile for sms+sdl
#
# Copyright (C) 1998, 1999, 2000  Charles Mac Donald
# SDL version by Gregory Montoir <cyx@frenchkiss.net>
#
# Define LSB_FIRST for little endian systems.

CC        = clang-3.6

NAME	  = sms_sdl
VERSION   = 0.9.3r5

ASMFLAGS  = -Wall -Werror
CFLAGS    = -I/usr/include/SDL -O2
DEFINES   = -DLSB_FIRST
INCLUDES  = -I. -I.. -Icpu
LIBS	  = -lSDL -lm

OBJECTS   = main.o saves.o sdlsms.o render.o sms.o system.o vdp.o  cpu/z80.o

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(NAME)

distrib: $(NAME)
	mkdir sms_sdl-$(VERSION); \
	cp $(NAME) README.txt sms_sdl-$(VERSION); \
	tar zcf sms_sdl-$(VERSION)-linux.tar.gz sms_sdl-$(VERSION)/*; \
	rm -rf sms_sdl-$(VERSION)
