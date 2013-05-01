# FileName	[ Makefile ]
#
# PackageName	[ doc ]
#
# Synopsis	[ Documentation package ]
#
# Description	[ Makefile template for the NuSMV's docs ]
#
# Author	[ Roberto Cavada ]
#
# Copyright	[Copyright (C) 1998-2001 by ITC-irst ]
#
# NuSMV version 2 is free software; you can redistribute it and/or 
# modify it under the terms of the GNU Lesser General Public 
# License as published by the Free Software Foundation; either 
# version 2 of the License, or (at your option) any later version.
#
# NuSMV version 2 is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public 
# License along with this library; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
#
# For more information of NuSMV see <http://nusmv.irst.itc.it>
# or email to <nusmv-users@irst.itc.it>.
# Please report bugs to <nusmv-users@irst.itc.it>.
#
# To contact the NuSMV development board, email to <nusmv@irst.itc.it>.]
#
# Revision	[Makefile.am,v 1.1.2.3 2004/03/18 16:17:35 nusmv Exp]

AUTOMAKE_OPTIONS = subdir-objects

html_files = $(shell find . -name '*.html' -print) 
dirs = $(shell find . -depth -type d -print | grep -v CVS | grep ./)
htmldatadir = $(datadir)/nusmv/doc/html
nobase_htmldata_DATA = $(dirs) $(html_files) 
txt_files = $(shell find $(top_builddir)/share/help -name '*.txt' -print) 

MOSTLYCLEANFILES = $(html_files) $(txt_files)


docs: helpfiles

helpfiles: 
	cd $(top_builddir) && $(SHELL) helpers/extract_doc .

install: docs install-am

clean: clean-am
	rm -rf $(dirs)
