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
# Revision	[1.1.2.2 2003/08/07 15:43:31]

INCLUDES = -I. -I.. -I$(top_srcdir)/src -I$(top_srcdir)/../cudd-2.3.0.1/include

BUILT_SOURCES = lex.c yacc.c yacc.h
MOSTLYCLEANFILES = $(BUILT_SOURCES)

YY_PREFIX = ltl2smv_yy
LEX_OUTPUT_ROOT = lex.$(YY_PREFIX)

AM_YFLAGS = -y -d -p$(YY_PREFIX)
AM_LFLAGS = -l -P$(YY_PREFIX)

ltl2smvdir = $(includedir)/nusmv/ltl/ltl2smv
ltl2smv_HEADERS = ltl2smv.h

LIBRARY_SOURCES = lex.l yacc.y ltl2smv.h ltl2smvInt.h ltl2smv.c
LTL2SMV_SOURCES = main.c


# -------------------------------------------------------

noinst_LTLIBRARIES = libltl2smv.la
libltl2smv_la_SOURCES =  $(LIBRARY_SOURCES)

# -------------------------------------------------------

bin_PROGRAMS = ltl2smv
ltl2smv_SOURCES = $(LTL2SMV_SOURCES) 
ltl2smv_LDADD = libltl2smv.la

# -------------------------------------------------------
