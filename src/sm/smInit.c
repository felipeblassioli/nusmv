/**CFile***********************************************************************

  FileName    [smInit.c]

  PackageName [sm]

  Synopsis    [Initializes and ends NuSMV.]

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``sm'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

  NuSMV version 2 is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "smInt.h"

#include "utils/error.h"
#include "utils/ustring.h"
#include "utils/assoc.h"
#include "utils/NodeList.h"
#include "trace/pkg_trace.h"

#include <string.h>

static char rcsid[] UTIL_UNUSED = "$Id: smInit.c,v 1.4.2.22.2.8 2005/11/16 12:09:47 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constants declarations                                                    */
/*---------------------------------------------------------------------------*/

#define CPP_NAME "cpp"
#define M4_NAME "m4"

/* number of fields in structure preprocessors_list: */
#define PP_FIELDS_NUM  3

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

FILE *nusmv_stderr;
FILE *nusmv_stdout;
FILE *nusmv_stdin;
FILE *nusmv_historyFile;
FILE *nusmv_stdpipe;
DdManager * dd_manager = (DdManager*) NULL;

FILE* def_nusmv_stderr;
FILE* def_nusmv_stdout;

options_ptr options = (options_ptr) NULL;
cmp_struct_ptr cmps = (cmp_struct_ptr) NULL;

/**Variable********************************************************************

  Synopsis    [The list of pre-processor names and how to call them.]

  Description [This array is used to store the names of the avaliable
pre-processors on the system, as well as the command to excecute them. The
names are stored in even indices, with the corresponding command stored and the
location immediately succeeding the name. The last two entries are NULL to
indicate that there are no more entries.]

  SeeAlso     []

******************************************************************************/
static char** preprocessors_list = (char**) NULL;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void init_preprocessors ARGS((void));
static void quit_preprocessors ARGS((void));

static char* get_executable_name ARGS((const char* command));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Shuts down and restarts the system]

  Description [Shuts down and restarts the system]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Sm_Reset()
{

  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr, "Shutting down the system...\n");
    inc_indent_size();
  }

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Clearing the flattener....");
  } 

  CompileFlatten_quit_flattener();

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Done\n");
  } 

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Clearing other compiler's hashes....");
  }
  
  clear_check_constant_hash();
  quit_expr2bexpr_hash();
  clear_wfftype_hash();
  clear_dependencies_hash();
  clear_define_dep_hash();
  clear_coi_hash();

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Done\n");
  }

  current_state_bdd_free();
  
  TracePkg_quit();
  PropPkg_quit();

  Enc_quit_encodings();

  if (global_fsm_builder != FSM_BUILDER(NULL)) {
    FsmBuilder_destroy(global_fsm_builder);
    global_fsm_builder = FSM_BUILDER(NULL);
  }

  /* reset rbc package */
  Bmc_Quit();

  #ifdef DEBUG
  if (opt_verbose_level_gt(options, 4)) {
    int result = dd_checkzeroref(dd_manager);
    if (result != 0) {
      fprintf(nusmv_stderr,
              "\n%d non-zero DD reference counts after dereferencing.\n", result);
    }
  }
  #endif


  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Clearing DD and node packages....");
  }

  quit_dd_package(dd_manager);
  node_quit();

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Done\n");
  }


  /* ====================================================================== */
  /*                          Reboot of the system                          */
  /* ====================================================================== */

  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr, "Shutdown completed, rebooting the system...\n");
  }

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Restarting the DD manager....");
  }

  node_init();
  dd_manager = init_dd_package();
  init_the_node();

  if (opt_verbose_level_gt(options, 2)) {
    fprintf(nusmv_stderr, "Done\n");
    fprintf(nusmv_stderr, "Restarting the compiler....");
  }
  cmps = cmp_struct_init();  /* reset all the flags */
  init_expr2bexp_hash(); 

  current_state_label_reset();
  current_state_bdd_reset();

  PropPkg_init();

  TracePkg_init();

  if (opt_verbose_level_gt(options, 2)) fprintf(nusmv_stderr, "Done\n");
  
  if (opt_verbose_level_gt(options, 1)) {
    fprintf(nusmv_stderr, "The system is now up and ready\n");
    dec_indent_size();      
  }
  
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Calls the initialization routines of all the packages.]

  SideEffects [Sets the global variables nusmv_stdout, nusmv_stderr,
  nusmv_historyFile.]

  SeeAlso     [SmEnd]

******************************************************************************/
void SmInit()
{
  init_memory();
  cmdFlagTable = avl_init_table(strcmp);
  nusmv_stdout     = stdout;
  nusmv_stderr     = stderr;
  nusmv_stdin      = stdin;
  def_nusmv_stdout = nusmv_stdout;
  def_nusmv_stderr = nusmv_stderr;
  nusmv_historyFile = NIL(FILE);

  /* win32 platforms seem to be "lazy" when they need to empy stream
     buffers. In this case we forse buffers to be emptied
     explicitly */
#if HAVE_SETVBUF && (defined(__MINGW32__) || defined(__CYGWIN__))
# if SETVBUF_REVERSED
      setvbuf(nusmv_stdout, _IOLBF, (char *) NULL, 0);
      setvbuf(nusmv_stderr, _IONBF, (char *) NULL, 0);
# else
      setvbuf(nusmv_stdout, (char *) NULL, _IOLBF, 0);
      setvbuf(nusmv_stderr, (char *) NULL, _IONBF, 0);
# endif
#endif

  init_string();
  node_init();

  init_preprocessors();

  init_param_hash();

  init_check_constant_hash();
  init_expr2bexp_hash();
  init_wfftype_hash();
  init_assign_db_hash();
  init_flatten_def_hash();
  init_flatten_constant_hash();
  init_dependencies_hash();
  init_define_dep_hash();
  init_coi_hash();
  dd_manager = init_dd_package();
  init_the_node();

  Cmd_Init();
  options = init_options();

  PropPkg_init();
  PropPkg_init_cmd();

  /* Other init here */
  sm_AddCmd();
  dd_AddCmd();
  Parser_Init();
  Compile_Init();

  Bdd_Init();
  Mc_Init();
  Ltl_Init();
  Simulate_Init();
  Bmc_AddCmd();

  TracePkg_init();
  traceCmd_init();
}


/**Function********************************************************************

  Synopsis    [Calls the end routines of all the packages.]

  SideEffects [Closes the output files if not the standard ones.]

  SeeAlso     [SmInit]

******************************************************************************/
void SmEnd()
{
  Cmd_End();
  Compile_End();

  if (global_fsm_builder != FSM_BUILDER(NULL)) {
    FsmBuilder_destroy(global_fsm_builder);
    global_fsm_builder = FSM_BUILDER(NULL);
  }

  Bdd_End();
  TracePkg_quit();
  PropPkg_quit_cmd();
  PropPkg_quit();
  Mc_End();
  Bmc_Quit();

  CompileFlatten_quit_flattener();

  quit_preprocessors();

  clear_check_constant_hash();
  quit_expr2bexpr_hash();
  clear_wfftype_hash();
  clear_dependencies_hash();
  clear_define_dep_hash();
  clear_coi_hash();

#ifdef DEBUG
  if (opt_verbose_level_gt(options, 4)) {
    int result = dd_checkzeroref(dd_manager);
    if (result != 0) {
      fprintf(stderr,"%d non-zero DD reference counts after dereferencing\n", result);
    }
  }
#endif

  quit_dd_package(dd_manager);  


  if (nusmv_stdout != stdout) fclose(nusmv_stdout);  
  if (nusmv_stderr != stderr) fclose(nusmv_stderr);
  if (nusmv_historyFile != NIL(FILE)) fclose(nusmv_historyFile);
  if (nusmv_stdin != stdin) fclose(nusmv_stdin);
  nusmv_stdout = stdout;
  nusmv_stderr = stderr;
  nusmv_stdin  = stdin;
  nusmv_historyFile = NIL(FILE);
}


/**Function********************************************************************

  Synopsis    [Initializes information about the pre-processors avaliable.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void init_preprocessors()
{  
  char* cpp_call = (char*) NULL;

  nusmv_assert(preprocessors_list == (char**) NULL);

  /* two triplets preprocessor/filename/command, one triplet for
     termination */
  preprocessors_list = ALLOC(char*, PP_FIELDS_NUM*3); 
  nusmv_assert(preprocessors_list != (char**) NULL);

  /* sets the executable file for cpp preprocessor */
#if HAVE_GETENV
  cpp_call = getenv("CPP");
#endif

  if (cpp_call == (char*) NULL) {
#if HAVE_CPP
    cpp_call = MACRO_STRINGIZE(PROG_CPP);
#else
    /* Tries anyway an executable: */
    cpp_call = CPP_NAME;
#endif
  }
  
  if (cpp_call == (char*) NULL) {
    internal_error("The pre-proprocessor could not be found.\n");
  }

  /* Stores the names of avaliable pre-processors along with the
     command to actually execute them. The NUL entries signify the end
     of the list: */

  /* cpp */
  preprocessors_list[0] = util_strsav(CPP_NAME); 
  preprocessors_list[1] = get_executable_name(cpp_call);
  preprocessors_list[2] = util_strsav(cpp_call);

  /* m4 */
  preprocessors_list[3] = util_strsav(M4_NAME);
  preprocessors_list[4] = get_executable_name(M4_NAME);
  preprocessors_list[5] = util_strsav(M4_NAME);

  /* terminators: */
  preprocessors_list[6] = (char*) NULL;
  preprocessors_list[7] = (char*) NULL;
  preprocessors_list[8] = (char*) NULL;
}


/**Function********************************************************************

  Synopsis [Given a command, returns the executable file name (with
  extension if required)]

  SideEffects [If not already specified, extension suffix is appended
  to the returned string. Returned string must be freed.]

  SeeAlso     []

******************************************************************************/
static char* get_executable_name(const char* command)
{
  char* space; 
  char* exec_name = (char*) NULL;
  size_t exec_len;
  size_t exeext_len;

  space = strchr(command, ' ');
  if (space != (char*) NULL) exec_len = (size_t) (space - command);
  else exec_len = strlen(command);

  exeext_len = strlen(EXEEXT);

  exec_name = ALLOC(char, exec_len + exeext_len + 1);
  nusmv_assert(exec_name != (char*) NULL);

  strncpy(exec_name, command, exec_len);
  exec_name[exec_len] = '\0'; /* adds a terminator */
  
  if ((exec_len > exeext_len) && (exeext_len > 0)) {
    /* the command might already contain EXEEXT: */
    char* pos; 
    pos = strstr(exec_name, EXEEXT);
    if ( (pos == (char*) NULL) || 
	 (((int) (pos - exec_name)) < (exec_len-exeext_len)) ) {
      /* add the suffix: */
      strcat(exec_name, EXEEXT);      
    }
  }
  else {
    /* it can't contain the suffix: add it */
    strcat(exec_name, EXEEXT);    
  }

  return exec_name;
}


/**Function********************************************************************

  Synopsis    [Removes information regarding the avaliable pre-processors.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void quit_preprocessors(void)
{
  char** iter;

  nusmv_assert(preprocessors_list != (char**) NULL);
  iter = preprocessors_list; 
  while (*iter != (char*) NULL) { FREE(*iter); }

  preprocessors_list = (char**) NULL;
}


/**Function********************************************************************

  Synopsis    [Gets the command line call for the specified pre-processor
  name. Returns NULL if given name is not available, or a string that must be 
  freed]

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* get_preprocessor_call(const char* name)
{
  char* res = (char*) NULL;
  char** iter;

  iter = preprocessors_list; 
  while (*iter != (char*) NULL) { 
    if (strncmp(*iter, name, strlen(name) + 1) == 0) {
      res = *(iter + 2);
      break;
    }
    
    iter += PP_FIELDS_NUM;
  }
  
  return (char*) res;  
}


/**Function********************************************************************

  Synopsis    [Gets the actual program name of the specified pre-processor.
  Returns NULL if given name is not available, or a string that must NOT be 
  freed]

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* get_preprocessor_filename(const char* name)
{
  char* res = (char*) NULL;
  char** iter;

  iter = preprocessors_list; 
  while (*iter != (char*) NULL) { 
    if (strncmp(*iter, name, strlen(name) + 1) == 0) {
      res = *(iter + 1);
      break;
    }
    
    iter += PP_FIELDS_NUM;
  }
  
  return (char*) res;  
}


/**Function********************************************************************

  Synopsis    [Returns the number of available proprocessors]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int get_preprocessors_num()
{
  int len = 0;
  char** iter;
  
  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    ++len;
    iter += PP_FIELDS_NUM;
  }

  return len;
}


/**Function********************************************************************

  Synopsis    [Gets the names of the avaliable pre-processors. Returned 
  string must be freed]

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* get_preprocessor_names()
{
  int len;
  char* names;
  char** iter;

  /* length of the string: */
  len = 0;
  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    len += strlen(*iter) + 1; /* for the additional space */
    iter += PP_FIELDS_NUM;
  }

  names = ALLOC(char, len+1);
  nusmv_assert(names != (char*) NULL);
  
  names[0] = '\0';
  iter = preprocessors_list;
  while (*iter != (char*) NULL) {
    strncat(names, *iter, strlen(*iter));
    strncat(names, " ", 1);
    iter += PP_FIELDS_NUM;
  }

  names[len] = '\0';
  return names;
}
