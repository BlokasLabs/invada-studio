#   Copyright (C) 2007 Fraser Stuart
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


#####################################################################
# PLEASE CHANGE THIS to your preferred installation location!
#
# Change this if you want to install somewhere else. In particular
# you may wish to remove the middle "local/" part of the path.

INSTALL_PLUGINS_DIR	=	/usr/local/lib/lv2/invada.lv2

# NO EDITING below this line is required
# if all you want to do is install and use the plugins.
#####################################################################


# GENERAL

CC		=	gcc
LD		=	ld
CFLAGS		=	-I. -O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -c -fPIC -DPIC
LDFLAGS		=	-shared -lc -lm -L. -linv_common

PLUGINS		=	libinv_common.a \
                        inv_input.so \
                        inv_tube.so \



all: $(PLUGINS)

# RULES TO BUILD PLUGINS FROM C CODE

libinv_common.a:   libinv_common.o  libinv_common.h     
inv_input.so:      inv_input.o      inv_input.h   
inv_tube.so:       inv_tube.o       inv_tube.h     
     


# OTHER TARGETS

install: targets
	-mkdir -p		$(INSTALL_PLUGINS_DIR)
	cp *.so 		$(INSTALL_PLUGINS_DIR)
	cp *.ttl 		$(INSTALL_PLUGINS_DIR)

targets:	$(PLUGINS)

always:	

clean:
	-rm -f `find . -name "*.so"`
	-rm -f `find . -name "*.a"`
	-rm -f `find . -name "*.o"`
	-rm -f `find .. -name "*~"`

%.o: %.c
	@echo "Compiling $<"
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 

%.so: %.o
	@echo "Creating  $@"
	@$(LD) -o $@ $< $(LDFLAGS)

%.a: %.o
	@echo "Creating  $@"
	@ar rcs $@ $<
