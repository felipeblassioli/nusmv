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
# Revision	[1.1.2.2 2003/07/31 15:41:06]

AUTOMAKE_OPTIONS = subdir-objects

MOSTLYCLEANFILES = nusmv.ps nusmv.pdf main.adx main.and main.aux \
		main.bbl main.blg main.dvi main.idx main.ilg main.ind \
		main.log main.ndx main.nnd main.toc

good_names = %.tex %.fig %.eps %.sty %.ist %.bib

cmd_files = $(filter $(good_names), $(wildcard cmd/*))
EXTRA_DIST = $(filter $(good_names), $(wildcard ./*)) $(cmd_files)


docs: nusmv.ps nusmv.pdf

nusmv.ps: app.tex batch.tex citeall.tex cmdpo.eps cmdpo.fig \
		epsf.tex index.sty inter.tex intro.tex main.bbl \
		main.tex nusmv.ist nusmv.sty syntax.tex title.tex \
		$(cmd_files)
	cd $(srcdir) && latex main.tex 
	dvips -o nusmv.ps $(srcdir)/main

nusmv.pdf: nusmv.ps
	ps2pdf nusmv.ps nusmv.pdf

main.bbl: main.bib
	cd $(srcdir) && latex main.tex && bibtex main && latex main.tex && \
	 makeindex -l -c -s nusmv.ist main && \
         makeindex -o  main.and main.adx && \
         makeindex  -o main.nnd main.ndx && \
	 latex main.tex 

if HAVE_LATEX
install-data-local: nusmv.ps
	$(install_sh_DATA) nusmv.ps $(datadir)/nusmv/doc/nusmv.ps
else 
install-data-local:
	@echo Cannot generate manual: latex not found
endif


uninstall-local: 
	test ! -f $(datadir)/nusmv/doc/nusmv.ps || \
	  rm -f $(datadir)/nusmv/doc/nusmv.ps


