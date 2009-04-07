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
CFLAGS		=	-I. -O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -c -fPIC -DPIC
LDFLAGS		=	-shared -Llibrary -lc -lm  -linv_common

PLUGINS		=	inv_compressor.so \
                        inv_erreverb.so \
                        inv_filter.so \
                        inv_input.so \
                        inv_tube.so \

all:	$(PLUGINS)
	

# RULES TO BUILD PLUGINS FROM C CODE

inv_compressor.so:	inv_compressor.o	inv_compressor.h
inv_erreverb.so:	inv_erreverb.o		inv_erreverb.h    
inv_filter.so:		inv_filter.o		inv_filter.h         
inv_input.so:		inv_input.o		inv_input.h   
inv_tube.so:		inv_tube.o		inv_tube.h     
     

# OTHER TARGETS

targets:	$(PLUGINS)

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

