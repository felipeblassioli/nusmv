# FileName	[ Makefile ]
#
# PackageName	[ NuSMV ]
#
# Synopsis	[ Package-wide Makefile ]
#
# Description	[ Makefile template for the NuSMV executable ]
#
# Author	[ Roberto Cavada ]
#
# Copyright	[Copyright (C) 2003 by ITC-irst ]
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
# Revision	[1.1.2.3 2003/08/07 15:43:31]


CUDD_NAME = $(cuddname)
CUDD_DIR = ../$(CUDD_NAME)

INCLUDES = -I. -I.. -I$(top_srcdir)/src -I$(top_srcdir)/$(CUDD_DIR)/include

SUBDIRS = ord psl

if HAVE_NOT_CUDD
BUILT_SOURCES = grammar.h grammar.c input.c $(top_srcdir)/$(CUDD_DIR)/include/cudd.h
else
BUILT_SOURCES = grammar.h grammar.c input.c 
endif

MOSTLYCLEANFILES = grammar.h grammar.c input.c

AM_YFLAGS = -d
AM_LFLAGS = -l

PKG_SOURCES = grammar.y input.l parser.h parserInt.h parserCmd.c \
	parserUtil.c symbols.h


parserdir = $(includedir)/nusmv/parser
parser_HEADERS =  parser.h symbols.h


# -------------------------------------------------------

noinst_LTLIBRARIES = libparser.la
libparser_la_SOURCES = $(PKG_SOURCES)

libparser_la_LIBADD = \
	ord/libparserOrd.la \
	psl/libparserpsl.la


$(top_srcdir)/$(CUDD_DIR)/include/cudd.h:
	cd $(top_srcdir)/$(CUDD_DIR) && $(SHELL) setup.sh
