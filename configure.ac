-*- mode: m4 -*-
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)

AC_INIT(NuSMV, 2.3.1, nusmv@irst.itc.it)

# where to look for install.sh and other helpers
AC_CONFIG_AUX_DIR(helpers)

AM_INIT_AUTOMAKE(NuSMV, 2.3.1)

AC_CANONICAL_HOST

AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER([config.h])

AC_DEFINE_UNQUOTED(PACKAGE_BUILD_DATE, "`date -u`", Build date)

cuddname=cudd-2.3.0.1
AC_SUBST(cuddname)


# Arguments

# libraries are static by default. User can override by passing 
# the flag --enable-dynamic
AC_DISABLE_SHARED


# If the PSL parser must be generated:
AC_ARG_ENABLE(psl,
  [  --enable-psl  Builds the executable pslparser to parse PSL properties (default is no)], 
  [if test "x$enableval" = "xyes"; then 
    enable_psl=yes
fi], [enable_psl=no])

AM_CONDITIONAL(HAVE_PSL_PARSER, test "x$enable_psl" = "xyes")


expatdefpath=/usr

# expat library dir:
AC_ARG_WITH(expat-libdir,
[  --with-expat-libdir=<dir> The directory which the expat library is contained in. 
			      (default is /usr/lib)],
[expatlibdir=$withval], [expatlibdir=$expatdefpath/lib])

# expat library header dir:
AC_ARG_WITH(expat-incdir,
  [  --with-expat-incdir=<dir> The directory which the expat.h header is contained in. 
			      (default are standard locations)],
  [ expatincdir=$withval
    if ! test -f $expatincdir/expat.h; then
        AC_MSG_ERROR(The expat.h header file has not been found in "$expatincdir".
See configure option --with-expat-incdir to specify where the header file is located.)
    else 
     expat_h_found=yes
     expatuserinc="-L $expatincdir"
    fi
  ], 
  [ expatuserinc=""
    AC_CHECK_HEADER(expat.h, 
      [expat_h_found=yes],
      [])
  ])
AC_SUBST(expatuserinc)


# SA package (no need to specify dir)

AC_ARG_ENABLE(sa,
[  --enable-sa    Turns on SA package inside NuSMV (default is no)],
[if test "x$enableval" = "xyes"; then 
    enable_sa=yes
fi], [enable_sa=no])

if test "x$enable_sa" = "xyes"; then 
   AC_DEFINE(HAVE_SA, 1, Enables SA package)
else # if not enable_sa
   AC_DEFINE(HAVE_SA, 0, Disables SA package)
fi

AM_CONDITIONAL(HAVE_SA, test "x$enable_sa" = "xyes")
AC_SUBST(HAVE_SA)
# see the file src/Makefile.am for further setting up of SA

# SA package: CPP flag
AC_ARG_ENABLE(sa_cpp,
[  --enable-sa-cpp    Turns on cpp use inside SA (default is no)],
[if test "x$enableval" = "xyes"; then 
    enable_sa_cpp=yes
fi], [enable_sa_cpp=no])

if test "x$enable_sa_cpp" = "xyes"; then 
   AC_DEFINE(HAVE_SA_CPP, 1, Enables cpp use inside SA)
else # if not enable_sa_cpp
   AC_DEFINE(HAVE_SA_CPP, 0, Disables cpp use inside SA)
fi

AM_CONDITIONAL(HAVE_SA_CPP, test "x$enable_sa_cpp" = "xyes")
AC_SUBST(HAVE_SA_CPP)


# ZChaff configuration
zchaff_nusmv=../zchaff
. $zchaff_nusmv/zchaff-default
AC_SUBST(zchaff_ver)
AC_SUBST(zchaff_nusmv)
AC_SUBST(zchaff_patch)
zchaffdefpath=$zchaff_link

# ZChaff library dir:
AC_SUBST(zchafflibdir)
AC_ARG_WITH(zchaff-libdir,
[  --with-zchaff-libdir=<dir> The directory which the zchaff library is contained in. 
			  Depends on --enable-zchaff. (default is ../zchaff.(version))],
[ if [ test -x $withval ]; then 
    zchafflibdir=`cd $withval && pwd`
  else
    zchafflibdir=$withval
  fi],
[ if [ test -x $zchaffdefpath ]; then 
    zchafflibdir=`cd $zchaffdefpath && pwd`
  else
    zchafflibdir=$zchaffdefpath
  fi]
)

# ZChaff include dir:
AC_SUBST(zchaffincdir)
AC_ARG_WITH(zchaff-incdir,
[  --with-zchaff-incdir=<dir> The directory the SAT.h header is contained in.
			  Depends on --enable-zchaff. (default is ../zchaff.(version))],
[ if [ test -x $withval ]; then 
    zchaffincdir=`cd $withval && pwd`
  else
    zchaffincdir=$withval
  fi],
[ if [ test -x $zchaffdefpath ]; then 
    zchaffincdir=`cd $zchaffdefpath && pwd`
  else
    zchaffincdir=$zchaffdefpath
  fi]
)


# If zchaff must be linked:
AC_ARG_ENABLE(zchaff,
  [  --enable-zchaff  Makes the ZChaff SAT solver usable from within NuSMV (default is no)], 
  [if test "x$enableval" = "xyes"; then 
    enable_zchaff=yes
fi], [enable_zchaff=no])


# MiniSat configuration
minisat_nusmv=../MiniSat
. $minisat_nusmv/minisat-default
AC_SUBST(minisat_ver)
AC_SUBST(minisat_nusmv)
minisatdefpath=$minisat_link

# MiniSat library dir:
AC_SUBST(minisatlibdir)
AC_ARG_WITH(minisat-libdir,
[  --with-minisat-libdir=<dir> The directory which the MiniSat library (libMiniSat.a) is contained in. 
			  Depends on --enable-minisat. (default is ../MiniSat_v1.14)],
[ if [ test -x $withval ]; then
    minisatlibdir=`cd $withval && pwd`
  else 
    minisatlibdir=$withval
  fi], 
[ if [ test -x $minisatdefpath ]; then
    minisatlibdir=`cd $minisatdefpath && pwd`
  else 
    minisatlibdir=$minisatdefpath
  fi]
)


# MiniSat include dir:
AC_SUBST(minisatincdir)
AC_ARG_WITH(minisat-incdir,
[  --with-minisat-incdir=<dir> The directory the interface files (Solver_C.h) is contained in.
			  Depends on --enable-minisat. (default is ../MiniSat_v1.14)],
[ if [ test -x $withval ]; then
    minisatincdir=`cd $withval && pwd`
  else
    minisatincdir=$withval
  fi], 
[ if  [ test -x $minisatdefpath ]; then 
    minisatincdir=`cd $minisatdefpath && pwd`
  else 
    minisatincdir=$minisatdefpath
  fi]
)


# If MiniSat must be linked:
AC_ARG_ENABLE(minisat,
  [  --enable-minisat  Makes the MiniSat SAT solver usable from within NuSMV (default is yes)], 
  [if test "x$enableval" = "xyes"; then 
    enable_minisat=yes
   fi
   if test "x$enableval" = "xno"; then 
    enable_minisat=no
   fi], [enable_minisat=auto])


# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP

if test "x$ac_cv_prog_CPP" != "x"; then
AC_DEFINE(HAVE_CPP, 1, Have preprocessor)
AC_DEFINE_UNQUOTED(PROG_CPP, $ac_cv_prog_CPP -x c, Preprocessor call string)
else
AC_DEFINE(HAVE_CPP, 0, Have preprocessor)
fi

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_YACC
AM_PROG_LEX

if test "$LEX" != flex; then
  LEX="$SHELL $missing_dir/missing flex"
  AC_SUBST(LEX_OUTPUT_ROOT, lex.yy)
  AC_SUBST(LEXLIB, '')
  if test "x$LEX" == "x"; then
    AC_MSG_ERROR("Working lex or flex are required to build NuSMV.")
  fi
fi

AC_PATH_PROG(PERL, perl, perl_is_missing)
AC_CHECK_PROGS(HTMLDUMP, lynx links, lynx_or_links_is_missing)

AC_PATH_PROG(LATEX, latex)
AM_CONDITIONAL(HAVE_LATEX, test "x$LATEX" != "x")

# Checks for libraries.
AC_CHECK_LIB([bsd], [main])

AC_CHECK_LIB([m], [exp])

# For readline:
# 1. chooses either ncurses, curses or termcap to be linked to readline
# 2. checks readline with the library selected in 1
# 3. uses the internal readline if 2 fails
AC_CHECK_LIB([ncurses], [main], [ac_termcap_lib=-lncurses], \
  [AC_CHECK_LIB([curses], [main], [ac_termcap_lib=-lcurses], \
     [AC_CHECK_LIB([termcap], [main], [ac_termcap_lib=-ltermcap], \
        [ac_termcap_lib=])])])

if test x$ac_termcap_lib == x; then
   AC_MSG_WARN(cannot find a library to link with readline (next step might fail))
fi

AC_CHECK_LIB([readline], [readline], \
   [AC_DEFINE(HAVE_LIBREADLINE, 1, Uses the system readline)
    readline_libs="-lreadline $ac_termcap_lib"], \
   [AC_DEFINE(HAVE_LIBREADLINE, 0, Uses the private readline)
    readline_libs="";\
    AC_MSG_WARN(cannot link readline: NuSMV will use an internal version)], \
   [$ac_termcap_lib])	
AC_SUBST(readline_libs)

# expat library
if test x$expat_h_found == xyes; then
LIBS_old=$LIBS
AC_CHECK_LIB(expat, XML_ParserCreate, 
   [ac_have_expat=yes
    AC_DEFINE(HAVE_LIBEXPAT, 1, Enables expat usage)
    expat_libs="-L$expatlibdir -lexpat"
    ],
   [ac_have_expat=no
    AC_DEFINE(HAVE_LIBEXPAT, 0, Disables expat usage)
    expat_libs=
    ], 
   [-L$expatlibdir])    
LIBS=$LIBS_old	

else 
  expat_libs=
  ac_have_expat=no
fi

AC_SUBST(expat_libs)		      

AM_CONDITIONAL(HAVE_LIBEXPAT, test "x$ac_have_expat" = "xyes")
AC_SUBST(HAVE_LIBEXPAT)


# checks for the cudd package:

LIBS_old=$LIBS
AC_CHECK_LIB(cudd_util, util_cpu_time, 
   [ac_have_cudd=yes], \
   [ac_have_cudd=no], \
   [-L../$cuddname/lib])

if test "x$ac_have_cudd" = "xyes"; then
   AC_CHECK_LIB(mtr, Mtr_AllocNode,
     [ac_have_cudd=yes], \
     [ac_have_cudd=no], \
     [-L../$cuddname/lib -lcudd_util])

   if test "x$ac_have_cudd" = "xyes"; then
     AC_CHECK_LIB(st, st_init_table,
       [ac_have_cudd=yes], \
       [ac_have_cudd=no], \
       [-L../$cuddname/lib -lmtr -lcudd_util])

     if test "x$ac_have_cudd" = "xyes"; then
       AC_CHECK_LIB(cudd, Cudd_Ref,
         [ac_have_cudd=yes], \
	 [ac_have_cudd=no], \
	 [-L../$cuddname/lib -lst -lmtr -lcudd_util])

     fi 
   fi 
fi
AM_CONDITIONAL(HAVE_NOT_CUDD, test "x$ac_have_cudd" = "xno")
LIBS=$LIBS_old	


#zchaff header:
if test "x$enable_zchaff" = "xyes"; then 
  if ! test -f $zchaffincdir/SAT.h; then
      	AC_MSG_ERROR(The SAT.h header file has not been found in "$zchaffincdir".
See configure option --with-zchaff-incdir to specify where the header file is located.)
  fi
fi

# zchaff library
ac_have_zchaff=no
if test "x$enable_zchaff" = "xyes"; then 
LIBS=$LIBS_old
AC_CHECK_LIB([sat], [SAT_AddClause], [ac_have_zchaff=yes], \
	[AC_MSG_ERROR(sat library not found in $zchafflibdir. 
 ZChaff's location can be specified with the option --with-zchaff-libdir=<dir>
 WARNING: If you are using gcc-3.x and you are trying to link with zchaff 
 version 2001.6.15 or previous you need to install a newer version of zchaff.
 Also you can configure NuSMV to be built with gcc-2.9.x. For example 
 by setting variable CC=gcc296 when invoking configure.)],\
	[-L$zchafflibdir -lstdc++])
LIBS=$LIBS_old
fi


AM_CONDITIONAL(HAVE_ZCHAFF, test "x$ac_have_zchaff" = "xyes")
if test "x$ac_have_zchaff" = "xyes"; then 
   AC_DEFINE(HAVE_SOLVER_ZCHAFF, 1, Enables ZChaff)

   # checks zchaff's version:
   CXXFLAGS_old=$CXXFLAGS
   CXXFLAGS="-I. -I$zchafflibdir"
   AC_LANG_PUSH(C++)
   zchaff_libs="-L$zchafflibdir -lsat -lstdc++"
   LIBS=$zchaff_libs
   AC_MSG_CHECKING(for sat library version)
   AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM([[#include "SAT.h"]], 
		[[void* mng = SAT_InitManager();
                  int lits[] = {1,2,3}; 
                  SAT_AddClause(mng, lits, 3, 0); 
 	          SAT_ReleaseManager(mng);]])], 
      [AC_MSG_RESULT(>= 2003.6.16 (new interface))
       AC_DEFINE(HAVE_OLD_ZCHAFF, 0, Uses the new version of zchaff)],

      [ #tries the old interface:
	AC_MSG_RESULT(Failed with newer interface)
	AC_MSG_CHECKING(for old interface of zchaff)
 	AC_COMPILE_IFELSE(
	  [AC_LANG_PROGRAM([[#include "SAT.h"]], 
		[[void* mng = SAT_InitManager();
                  int lits[] = {1,2,3}; 
                  SAT_AddClause(mng, lits, 3); 
 	          SAT_ReleaseManager(mng);]])],
	  [AC_MSG_RESULT(< 2003.6.16 (old interface))
           AC_DEFINE(HAVE_OLD_ZCHAFF, 1, Uses the old version of zchaff)], 
	  [ AC_MSG_RESULT(Failed the older as well)
	    AC_MSG_ERROR([Could not find a compilable version of zchaff. 
See config.log for details. 
If you are trying to link the an old version of zchaff, it might the case you 
use an old version of gcc like gcc296 for example. To specify the C compiler to be 
used, pass for example CC=gcc296 to configure when you invoke it. 
If you cannot manage with this, contact NuSMV development team for futher help.]) ])
     ])
   LIBS=$LIBS_old
   CXXFLAGS=$CXXFLAGS_old
   AC_LANG_POP(C++)

else # if not ac_have_zchaff
   AC_DEFINE(HAVE_SOLVER_ZCHAFF, 0, Disables ZChaff)
   zchaff_libs=
fi # if ac_have_zchaff

AC_SUBST(zchaff_libs)
AC_SUBST(HAVE_ZCHAFF)

# MiniSat header:
if test "x$enable_minisat" = "xyes"; then 
  if ! test -f $minisatincdir/Solver_C.h; then
      	AC_MSG_ERROR(The Solver_C.h header file has not been found in "$minisatincdir".
See configure option --with-minisat-incdir to specify where the header file is located.)
  fi
fi

if test "x$enable_minisat" = "xauto"; then 
  if ! test -f $minisatincdir/Solver_C.h; then
      	AC_MSG_WARN(The Solver_C.h header file has not been found in "$minisatincdir".
See configure option --with-minisat-incdir to specify where the header file is located.
Minisat will not be linked to the system.)
        enable_minisat=no
  fi
fi

# MiniSat library
ac_have_minisat=no
if test "x$enable_minisat" = "xyes"; then 
LIBS=$LIBS_old
AC_CHECK_LIB([MiniSat], [MiniSat_Create],\
        [ac_have_minisat=yes], \
	[AC_MSG_ERROR(MiniSat library not found in $minisatlibdir
 Minisat's location can be specified with the option --with-minisat-libdir=<dir>.)],\
	[-L$minisatlibdir -lstdc++])
LIBS=$LIBS_old
fi

if test "x$enable_minisat" = "xauto"; then 
LIBS=$LIBS_old
AC_CHECK_LIB([MiniSat], [MiniSat_Create],\
        [ac_have_minisat=yes], \
	[AC_MSG_WARN(MiniSat library not found in $minisatlibdir 
 Minisat's location can be specified with the option --with-minisat-libdir=<dir>.
 Minisat will NOT be linked to NuSMV.)
         enable_minisat=no
         ac_have_minisat=no],\
	[-L$minisatlibdir -lstdc++])
LIBS=$LIBS_old
fi


AM_CONDITIONAL(HAVE_MINISAT, test "x$ac_have_minisat" = "xyes")
if test "x$ac_have_minisat" = "xyes"; then 
   AC_DEFINE(HAVE_SOLVER_MINISAT, 1, Enables Minisat)
   minisat_libs="-L$minisatlibdir -lMiniSat  -lstdc++"
else # if not ac_have_minisat
   AC_DEFINE(HAVE_SOLVER_MINISAT, 0, Disables Minisat)
   minisat_libs=
fi # if ac_have_minisat

AC_SUBST(minisat_libs)
AC_SUBST(HAVE_MINISAT)


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h limits.h memory.h stddef.h stdlib.h string.h sys/ioctl.h sys/param.h sys/time.h unistd.h signal.h sys/signal.h])

# This is for Linux
AC_CHECK_HEADERS(bsd/sgtty.h)

# This is for malloc:
AC_CHECK_HEADER(sys/types.h)
AC_CHECK_HEADER(malloc.h, 
	[ AC_DEFINE(HAVE_MALLOC_H, 1, Defined to 1 if the system provides malloc.h)],
	[ AC_CHECK_HEADER(sys/malloc.h, 
	    [AC_DEFINE(HAVE_SYS_MALLOC_H, 1, Defined to 1 if the system provides sys/malloc.h)],
	    [],
	    [
	      #if HAVE_SYS_TYPES_H
	      # include <sys/types.h>
	      #endif
	    ]  
	    )],
	[
	 #if HAVE_SYS_TYPES_H
	 # include <sys/types.h>
	 #endif
	])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor memmove memset pow strcasecmp strchr \
		strrchr strstr strtol, random srandom getpid \
		mkstemp getenv setvbuf])

# Executable file names suffix:
AC_DEFINE_UNQUOTED(EXEEXT, "$EXEEXT", Executable file names suffix)


AC_CONFIG_FILES([Makefile
		 src/Makefile
                 src/be/Makefile
                 src/bmc/Makefile
                 src/cmd/Makefile
                 src/compile/Makefile
                 src/dag/Makefile
                 src/dd/Makefile
                 src/enc/Makefile
                 src/fsm/Makefile
                 src/ltl/Makefile
                 src/ltl/ltl2smv/Makefile
                 src/mc/Makefile
                 src/node/Makefile
                 src/opt/Makefile
                 src/parser/Makefile
                 src/parser/ord/Makefile
                 src/parser/psl/Makefile
                 src/prop/Makefile
                 src/rbc/Makefile
                 src/sat/Makefile
                 src/sat/solvers/Makefile
                 src/set/Makefile
                 src/sim/Makefile
                 src/simulate/Makefile
                 src/sm/Makefile
                 src/trace/Makefile
                 src/trans/Makefile
                 src/utils/Makefile
		 doc/Makefile
		 doc/user-man/Makefile
     		 doc/tutorial/Makefile
     		 doc/html/Makefile
     		 examples/Makefile
		 share/Makefile
		 contrib/Makefile
  		 helpers/extract_doc
		 nusmv-2.2.pc])
AC_OUTPUT

echo


echo ------------------------------------------------------------------ 
if test "x$ac_have_zchaff" = "xyes"; then 
echo "The ZCHAFF library will be embedded into the NuSMV executable."
else 
echo "The ZCHAFF sat solver will NOT be linked to NuSMV."
echo "If you want to link it, please use configure options"
echo "--enable-zchaff, --with-zchaff-libdir, --with-zchaff-incdir."
echo "For further help, try \"./configure --help\"."
fi
echo ------------------------------------------------------------------ 
echo

echo ------------------------------------------------------------------ 
if test "x$ac_have_minisat" = "xyes"; then 
echo "The MINISAT library will be embedded into the NuSMV executable."
else 
echo "The MINISAT sat solver will NOT be linked to NuSMV."
echo "If you want to link it, please use configure options"
echo "--enable-minisat, --with-minisat-libdir, --with-minisat-incdir."
echo "For further help, try \"./configure --help\"."
fi
echo ------------------------------------------------------------------ 
echo

if test "x$enable_sa" = "xyes"; then 
echo ------------------------------------------------------------------ 
echo "This version of NuSMV will be compiled with SA support."
echo ------------------------------------------------------------------ 
echo
fi

if test "x$expat_h_found" = "xno"; then
echo ------------------------------------------------------------------ 
echo "The expat library has not been found.                            "
echo "See configure options --with-expat-{inc,lib}dir to help "
echo "configure finding expat if it is installed in non-standard paths." 
echo ------------------------------------------------------------------ 
echo
fi

if test "x$ac_have_expat" = "xno"; then 
echo ------------------------------------------------------------------ 
echo "The expat library has not been found.                            "
echo "It might be the case that either libexpat is not installed on    "
echo "your system, or it is actually installed, but configure could not"
echo "find it. See configure options --with-expat-{inc,lib}dir to help "
echo "configure finding expat if it is installed in non-standard paths." 
echo "**  If you run 'make' now, NuSMV would still build correctly   **"
echo "**  but the functionality to load saved traces from a file     **"
echo "**  will not be available.                                     **"
echo ------------------------------------------------------------------ 
echo
fi


if test "x$ac_have_cudd" = "xno"; then
echo "Warning: Cannot find the cudd libraries. Did you build cudd?"
echo "         If you did not, make will search for cudd dir into"
echo "         the parent dir, and will make cudd before making"
echo "         the NuSMV executable"
echo
fi

if test "x$enable_psl" = "xyes"; then
echo ------------------------------------------------------------------ 
echo "As you requested, the PSL parser will be generated as well."
echo ------------------------------------------------------------------ 
echo
fi
