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


GLADES		=	inv_compressor_gui.xml \
			inv_filter_gui.xml \
			inv_input_gui.xml \
			inv_tube_gui.xml \


all:	$(GLADES)

# RULES TO BUILD XML 

inv_compressor_gui.xml:	inv_compressor_gui.glade
inv_filter_gui.xml:	inv_filter_gui.glade
inv_input_gui.xml:	inv_input_gui.glade
inv_tube_gui.xml:	inv_tube_gui.glade


# OTHER TARGETS


targets:	$(GLADES)

always:	

clean:
	-rm -f *.xml
	-rm -f *~

%.xml: %.glade
	@echo "Generating $@"
	@gtk-builder-convert $< $@ 


