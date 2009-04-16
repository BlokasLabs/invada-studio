#   Copyright (C) 2009 Fraser Stuart
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#   


# GENERAL

CC		=	gcc
LD		=	ld
CFLAGS		=	-I. `pkg-config --cflags gtk+-2.0`  -O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -c -fPIC -DPIC
LDFLAGS		=	-shared -Lwidgets -lc -lm -linv_widgets


GUIS		=	inv_filter_gui.so \
			inv_input_gui.so \
			inv_tube_gui.so \


all:	$(GUIS) 
     
# RULES TO BUILD GUIS FROM C CODE
 
inv_filter_gui.so:     inv_filter_gui.o	../plugin/inv_filter.h	inv_filter_gui.h   
inv_input_gui.so:     inv_input_gui.o	../plugin/inv_input.h	inv_input_gui.h   
inv_tube_gui.so:     inv_tube_gui.o	../plugin/inv_tube.h	inv_tube_gui.h 


# OTHER TARGETS

targets: $(GUIS)

always:	

clean:
	-rm -f *.so
	-rm -f *.o
	-rm -f *~

%.o: %.c
	@echo "Compiling $<"
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 

%.so: %.o
	@echo "Creating  $@"
	@$(LD) -o $@ $< $(LDFLAGS)

