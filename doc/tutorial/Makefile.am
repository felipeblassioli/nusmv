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

MOSTLYCLEANFILES = tutorial.ps tutorial.pdf main.adx main.and main.aux \
		main.bbl main.blg main.dvi main.idx main.ilg main.ind \
		main.log main.ndx main.nnd main.toc

good_names = %.tex %.fig %.eps %.sty %.ist %.bib
EXTRA_DIST = $(filter $(good_names), $(wildcard ./*))

docs: tutorial.ps tutorial.pdf

tutorial.ps: bmc.tex citeall.tex ctl_mc.tex examples.tex intro.tex \
		ltl_mc.tex main.bbl main.tex nusmv.ist nusmvtut.sty \
		simulation.tex title.tex
	cd $(srcdir) && latex main.tex
	dvips -o tutorial.ps $(srcdir)/main

tutorial.pdf: tutorial.ps
	ps2pdf tutorial.ps tutorial.pdf


main.bbl: main.bib
	cd $(srcdir) && latex main.tex && bibtex main && latex main.tex && \
	  makeindex -l -c  -s nusmv.ist main && \
	  makeindex -o  main.and main.adx && \
	  makeindex  -o main.nnd main.ndx && \
	  latex main.tex

if HAVE_LATEX
install-data-local: tutorial.ps
	$(install_sh_DATA) tutorial.ps $(datadir)/nusmv/doc/tutorial.ps

else 
install-data-local:
	@echo Cannot generate tutorial: latex not found
endif

uninstall-local: 
	test ! -f $(datadir)/nusmv/doc/tutorial.ps || \
           rm -f $(datadir)/nusmv/doc/tutorial.ps
