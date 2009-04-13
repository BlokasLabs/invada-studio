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


#####################################################################
# PLEASE CHANGE THIS to your preferred installation location!
#
# Change this if you want to install somewhere else. 

INSTALL_SYS_PLUGINS_DIR		=	/usr/local/lib/lv2
INSTALL_USER_PLUGINS_DIR	=	~/.lv2
INSTALL_BUNDLE_DIR		=	invada.lv2

# NO EDITING below this line is required
# if all you want to do is install and use the plugins.
#####################################################################


# GENERAL

SUBDIRS		=	plugin/library plugin plugingui/widgets plugingui
GLADEDIRS	=	plugingui/gtk 


all:	        
	@for i in $(SUBDIRS); do \
        echo "\nmake all in $$i..."; \
        (cd $$i; $(MAKE) ); done

glade:	        
	@for i in $(GLADEDIRS); do \
        echo "\nmake all in $$i..."; \
        (cd $$i; $(MAKE) ); done


# OTHER TARGETS

install:
	@echo ""
	@echo "use 'make install-user' to install in $(INSTALL_USER_PLUGINS_DIR) or 'make install-sys' to install in $(INSTALL_SYS_PLUGINS_DIR)"
	@echo ""

install-sys: 
	-mkdir -p		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	-mkdir -p		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk
	cp plugin/*.so 		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp plugingui/*.so	$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp rdf/*.ttl 		$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp plugingui/gtk/*.png	$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk
	cp plugingui/gtk/*.xml	$(INSTALL_SYS_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk

install-user: 
	-mkdir -p		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	-mkdir -p		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk
	cp plugin/*.so 		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp plugingui/*.so	$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp rdf/*.ttl 		$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)
	cp plugingui/gtk/*.png	$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk
	cp plugingui/gtk/*.xml	$(INSTALL_USER_PLUGINS_DIR)/$(INSTALL_BUNDLE_DIR)/gtk


always:	

clean:
	-rm -f `find . -name "*.so"`
	-rm -f `find . -name "*.a"`
	-rm -f `find . -name "*.o"`
	-rm -f `find . -name "*~"`

veryclean:
	-rm -f `find . -name "*.so"`
	-rm -f `find . -name "*.a"`
	-rm -f `find . -name "*.o"`
	-rm -f `find . -name "*.xml"`
	-rm -f `find . -name "*~"`


