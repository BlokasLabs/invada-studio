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

INSTALL_SYS_PLUGINS_DIR		=	/usr/local/lib/lv2
INSTALL_USER_PLUGINS_DIR	=	~/.lv2
INSTALL_BUNDLE_DIR		=	invada.lv2

# NO EDITING below this line is required
# if all you want to do is install and use the plugins.
#####################################################################


# GENERAL

CC		=	gcc
LD		=	ld
CFLAGS		=	-I. -I/usr/include/libgnome-2.0 -I/usr/include/orbit-2.0 -I/usr/include/gconf/2 -I/usr/include/gnome-vfs-2.0 -I/usr/lib/gnome-vfs-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libbonobo-2.0 -I/usr/include/bonobo-activation-2.0 -I/usr/include/libgnomeui-2.0 -I/usr/include/libart-2.0 -I/usr/include/gnome-keyring-1 -I/usr/include/libbonoboui-2.0 -I/usr/include/libgnomecanvas-2.0 -I/usr/include/gtk-2.0 -I/usr/include/libxml2 -I/usr/include/pango-1.0 -I/usr/include/gail-1.0 -I/usr/include/freetype2 -I/usr/include/atk-1.0 -I/usr/lib/gtk-2.0/include -I/usr/include/cairo -I/usr/include/libpng12 -I/usr/include/pixman-1 -I/usr/include/libglade-2.0  -O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -c -fPIC -DPIC
LDFLAGS		=	-shared -lc -lm -L. -linv_common

PLUGINS		=	libinv_common.a \
                        inv_compressor.so \
                        inv_erreverb.so \
                        inv_filter.so \
                        inv_input.so \
                        inv_tube.so \

GUIS		=	inv_filter_gui.so \


all: $(PLUGINS) \
     $(GUIS)

# RULES TO BUILD PLUGINS FROM C CODE

libinv_common.a:   libinv_common.o  libinv_common.h 
inv_compressor.so: inv_compressor.o inv_compressor.h
inv_erreverb.so:   inv_erreverb.o   inv_erreverb.h    
inv_filter.so:     inv_filter.o     inv_filter.h         
inv_input.so:      inv_input.o      inv_input.h   
inv_tube.so:       inv_tube.o       inv_tube.h     
     
# RULES TO BUILD GUIS FROM C CODE
 
inv_filter_gui.so:     inv_filter_gui.o     inv_filter.h     inv_filter_gui.h   


# OTHER TARGETS

install:
	@echo ""
	@echo "use 'make install-user' to install in $(INSTALL_USER_PLUGINS_DIR) or 'make install-sys' to install in $(INSTALL_SYS_PLUGINS_DIR)"
	@echo ""

install-sys: targets
	-mkdir -p		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	-mkdir -p		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/images
	-mkdir -p		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk
	cp *.so 		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp rdf/*.ttl 		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp images/*.png		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/images
	cp gtk/*.glade 		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk

install-user: targets
	-mkdir -p		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	-mkdir -p		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/images
	-mkdir -p		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk
	cp *.so 		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp rdf/*.ttl 		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp images/*.png		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/images
	cp gtk/*.glade 		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk


targets:	$(PLUGINS) \
                $(GUIS)

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
