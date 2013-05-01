# FileName	[ Makefile ]
#
# PackageName	[ NuSMV ]
#
# Synopsis	[ Package-wide Makefile ]
#
# Description	[ Makefile template for the NuSMV library ]
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
# Revision	[1.1.2.4 2003/08/04 11:29:51]

## parser must be the first package to be built
SUBDIRS = parser be cmd dag enc ltl mc opt prop sat set \
	simulate trans trace bmc compile dd fsm node    \
	rbc sim sm utils 


## -------------------------------------------------------
## The NuSMV's library:

lib_LTLIBRARIES = libnusmvcore.la	

libnusmvcore_la_SOURCES = 
libnusmvcore_la_LIBADD = \
	parser/libparser.la \
	utils/libutils.la   \
	dag/libdag.la       \
	set/libset.la       \
	node/libnode.la     \
	rbc/librbc.la       \
	enc/libenc.la       \
	dd/libdd.la         \
	fsm/libfsm.la       \
	trans/libtrans.la   \
	trace/libtrace.la   \
	mc/libmc.la         \
	ltl/libltl.la       \
	sat/libsat.la       \
	be/libbe.la         \
	bmc/libbmc.la       \
	sim/libsim.la       \
	simulate/libsimulate.la  \
	cmd/libcmd.la            \
	opt/libopt.la            \
	prop/libprop.la          \
	compile/libcompile.la    \
	sm/libsm.la 
