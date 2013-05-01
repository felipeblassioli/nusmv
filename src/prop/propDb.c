/**CFile***********************************************************************

  FileName    [propDb.c]

  PackageName [prop]

  Synopsis    [Property database]

  Description [Primitives to create, query and manipulate a database
  of property is provided.]

  SeeAlso     []

  Author      [Roberto Cavada, Marco Roveri]

  Copyright   [
  This file is part of the ``prop'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by ITC-irst. 

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

  For more information on NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

******************************************************************************/

#include "propInt.h" 
#include "prop/prop.h"

#include "utils/error.h"
#include "parser/symbols.h"
#include "parser/parser.h"
#include "parser/psl/pslNode.h"

#include "utils/ucmd.h"
#include "utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static array_t* global_prop_database = (array_t*) NULL;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int 
prop_db_prop_parse_from_arg_and_add ARGS((Encoding_ptr senc, 
					  int argc, char** argv, 
					  const Prop_Type type));

static const char* prop_db_get_prop_type_as_parsing_string 
ARGS((const Prop_Type type));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Initializes the DB of properties]

  Description        [Initializes the DB of properties to an empty DB]

  SideEffects        []

******************************************************************************/
void PropDb_create() 
{
  global_prop_database = array_alloc(Prop_ptr, 1);

  if (global_prop_database == (array_t*) ARRAY_OUT_OF_MEM) {
    fprintf(nusmv_stderr, "PropDb_create: Unable to initialize Properties DB.\n");
    nusmv_exit(1);
  }
}


/**Function********************************************************************

  Synopsis           [Disposes the DB of properties]

  Description        [Disposes the DB of properties]

  SideEffects        []

******************************************************************************/
void PropDb_destroy() 
{
  int i;
  int num;
  Prop_ptr prop;
  
  num = PropDb_get_size();
  for (i = 0; i < num; ++i) {
    prop = PropDb_get_prop_at_index(i);
    Prop_destroy(prop);
  }

  array_free(global_prop_database);
}


/**Function********************************************************************

  Synopsis           [Fills the DB of properties]

  Description        [Given for each kind of property a list of
  respective fomulae, this function is responsible to fill the DB with
  them. Returns 1 if an error occurred, 0 otherwise]

  SideEffects        []

******************************************************************************/
int PropDb_fill(Encoding_ptr senc, node_ptr ctlspec, 
		node_ptr computespec, node_ptr ltlspec, node_ptr pslspec, 
		node_ptr invarspec)
{
  node_ptr l;
  int res; 

  for(l = ctlspec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(senc, car(l), Prop_Ctl);
    if (res == -1) return 1;
  }
  for(l = computespec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(senc, car(l), Prop_Compute);
    if (res == -1) return 1;
  }
  for(l = ltlspec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(senc, car(l), Prop_Ltl);
    if (res == -1) return 1;
  }
  for(l = pslspec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(senc, car(l), Prop_Psl);
    if (res == -1) return 1;
  }
  for(l = invarspec; l != Nil; l = cdr(l)) {
    res = PropDb_prop_create_and_add(senc, car(l), Prop_Invar);
    if (res == -1) return 1;
  }

  return 0;
}


/**Function********************************************************************

  Synopsis           [Inserts a property in the DB of properties]

  Description        [Insert a property in the DB of properties. 
  Returns true if out of memory]

  SideEffects        []

******************************************************************************/
boolean PropDb_add(Prop_ptr p) 
{
  boolean res;
  nusmv_assert(global_prop_database != (array_t*) NULL);

  res = (array_insert_last(Prop_ptr, global_prop_database, p) == ARRAY_OUT_OF_MEM);
  return res;;
}

/**Function********************************************************************

  Synopsis           [Inserts a property in the DB of properties]

  Description        [Given a formula and its type, a property is
  created and stored in the DB of properties. It returns either -1 in
  case of failure, or the index of the inserted property.]

  SideEffects        []

******************************************************************************/
int PropDb_prop_create_and_add(Encoding_ptr senc, node_ptr spec, 
			       Prop_Type type)
{
  int retval = 0;
  int index = PropDb_get_size();
  boolean allow_adding = true;
  boolean allow_checking = true;
  Prop_ptr prop = NULL; 

  boolean is_ctl = (type == Prop_Ctl);
  node_ptr spec_to_check;

  /* PSL properties need to be converted to CTL or LTL
     specifications */
  if (type == Prop_Psl) {
    PslNode_ptr psl_prop = PslNode_convert_from_node_ptr(spec);
    /* removal of forall */
    psl_prop = PslNode_remove_forall_replicators(psl_prop);    
    if (PslNode_is_handled_psl(psl_prop)) {
      /* either smooth LTL or SERE: */
      if (!PslNode_is_ltl(psl_prop)) {
	/* here it is a SERE: must be converted to LTL */
	psl_prop = PslNode_remove_sere(psl_prop);
      }

      /* converts to SMV ltl */
      spec_to_check = PslNode_pslltl2ltl(psl_prop, PSL2SMV);
    }      
    else {
      /* here the property may be either OBE or unmanageable */
      if (PslNode_is_obe(psl_prop)) {
	spec_to_check = PslNode_pslobe2ctl(psl_prop, PSL2SMV);
	is_ctl = true;
      }
      else {
	/* it is not supported */
	warning_psl_not_supported_feature(spec, index);
	allow_checking = false;
      }
    }
  }
  else spec_to_check = spec;  /* not PSL */

  if (allow_checking) {
    CATCH {
      Compile_FlattenSexpExpandDefine(senc, spec_to_check, Nil);
    }
    FAIL {
      retval = -1;
    }
    
    if (retval == -1) return retval;  /* undefined symbols */
  }

  prop = Prop_create_partial(spec, type);
  Prop_set_index(prop, index);

  if (allow_checking) {
    /* Checks for input vars */
    if (is_ctl || (type == Prop_Invar) || (type == Prop_Compute)) {
      NodeList_ptr expr_vars;

      if (opt_verbose_level_gt(options, 5)) {
	fprintf(nusmv_stdout,
		"Checking %s property (index %d) for input variables. \n",
		Prop_get_type_as_string(prop), index);
      }

      /* Get list of variables in the expression, and check for inputs */
      expr_vars = Compile_get_expression_dependencies(senc, Prop_get_expr(prop));
      allow_adding = !Encoding_list_contains_input_vars(senc, expr_vars);
      NodeList_destroy(expr_vars);
    }
  }

  /* If no input vars present then add property to database */
  if (allow_adding) {
    if (opt_verbose_level_gt(options, 3)) {
      fprintf( nusmv_stdout, 
	       "Attempting to add %s property (index %d) to property list.\n", 
	       Prop_get_type_as_string(prop), index);
    }
    retval = PropDb_add(prop);

    if (opt_verbose_level_gt(options, 3)) {
      if (retval == 1) {
	fprintf(nusmv_stdout, \
		"Failing to add %s property (index %d) to property list.\n", \
		Prop_get_type_as_string(prop), index);
      }
      else {
	fprintf(nusmv_stdout, \
		"%s property (index %d) successfully added to property list.\n",\
		Prop_get_type_as_string(prop), index);
      }
    }
  }
  else { 
    /* Property contains input variables */
    error_property_contains_input_vars(prop);
  }

  retval = (retval == 1) ? -1 : index;
  return retval;
}


/**Function********************************************************************

  Synopsis           [Returns the last entered property in the DB]

  Description        [Returns the last entered property in the DB of properties.]

  SideEffects        []

******************************************************************************/
Prop_ptr PropDb_get_last() 
{
  Prop_ptr res; 
  nusmv_assert(global_prop_database != (array_t*) NULL);

  res = array_fetch_last(Prop_ptr, global_prop_database); 
  return res;
}

/**Function********************************************************************

  Synopsis           [Returns the property indexed by index]

  Description        [Returns the property whose unique identifier is
  provided in input.last entered property in the DB of properties.]

  SideEffects        []

******************************************************************************/
Prop_ptr PropDb_get_prop_at_index(const int index) 
{
  Prop_ptr res; 
  nusmv_assert(global_prop_database != (array_t*) NULL);

  if (index >= array_n(global_prop_database)) res = PROP(NULL);
  else res = array_fetch(Prop_ptr, global_prop_database, index);

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the index of a property in the database]

  Description        [Returns the unique identifier of a property. It
  is used in the database of property to identify properties.]

  SideEffects        []

******************************************************************************/
int PropDb_get_prop_index(const Prop_ptr property)
{
  return Prop_get_index(property);
}


/**Function********************************************************************

  Synopsis           [Returns the size of the DB]

  Description        [Returns the size (i.e. the number of entries)
  stored in the DB of properties.]

  SideEffects        []

******************************************************************************/
int PropDb_get_size() 
{
  nusmv_assert(global_prop_database != (array_t*) NULL);

  return array_n(global_prop_database);
}


/**Function********************************************************************

  Synopsis           [Prints the header of the property list]

  Description        [optional]

  SideEffects        []

******************************************************************************/
void PropDb_print_list_header(FILE* file)
{
  fprintf(file, "**** PROPERTY LIST [ Type, Status, Counterex. Trace ] ****\n");
  fprintf(file, "--------------------------  PROPERTY LIST  -------------------------\n");
}


/**Function********************************************************************

  Synopsis           [Prints the specified property from the DB]

  Description        [Prints on the given file stream the property
  whose unique identifier is specified]

  SideEffects        []

******************************************************************************/
int PropDb_print_prop_at_index(FILE* file, const int index)
{
  int retval = 0;
  Prop_ptr prop = PropDb_get_prop_at_index(index);

  if (prop != PROP(NULL)) {
    Prop_print_db(prop, file);
  } else {
    retval = 1;
  }
  return retval;
}

/**Function********************************************************************

  Synopsis           [Prints all the properties stored in the DB]

  Description        [Prints on the given file stream all the property
  stored in the DB of properties.]

  SideEffects        []

******************************************************************************/
void PropDb_print_all(FILE* file)
{
  int i;
  int size;

  nusmv_assert(global_prop_database != (array_t*) NULL);

  size = PropDb_get_size();
  for (i = 0; i < size; ++i) {
    PropDb_print_prop_at_index(file, i);
  }

  if (i == 0) {
    fprintf(file, "The properties DB is empty.\n");
  }
}


/**Function********************************************************************

  Synopsis           [Prints all the properties stored in the DB]

  Description        [Prints on the given file stream all the property
  stored in the DB of properties whose type and status match the
  requested ones.]

  SideEffects        []

******************************************************************************/
void PropDb_print_all_status_type(FILE* file, Prop_Status status, 
				  Prop_Type type)
{
  int i, s;

  s = PropDb_get_size();
  for (i = 0; i < s; ++i) {
    Prop_ptr p = PropDb_get_prop_at_index(i);

    if ((Prop_get_type(p) == type) && (Prop_get_status(p) == status)) {
      Prop_print_db(p, file);
    }
  }
}

/**Function********************************************************************

  Synopsis           [Prints all the properties stored in the DB]

  Description        [Prints on the given file stream all the property
  stored in the DB of properties whose type match the requested one.]

  SideEffects        []

******************************************************************************/
void PropDb_print_all_type(FILE* file, Prop_Type type)
{
  int i, s;

  s = PropDb_get_size();
  for (i = 0; i < s; ++i) {
    Prop_ptr p = PropDb_get_prop_at_index(i);

    if (Prop_get_type(p) == type) {
      Prop_print_db(p, file);
    }
  }
}

/**Function********************************************************************

  Synopsis           [Prints all the properties stored in the DB]

  Description        [Prints on the given file stream all the property
  stored in the DB of properties whose status match the requested one.]

  SideEffects        []

******************************************************************************/
void PropDb_print_all_status(FILE* file, Prop_Status status)
{
  int i, s;

  s = PropDb_get_size();
  for (i = 0; i < s; ++i) {
    Prop_ptr p = PropDb_get_prop_at_index(i);

    if (Prop_get_status(p) == status) {
      Prop_print_db(p, file);
    }
  }
}


/**Function********************************************************************

  Synopsis           [Return the list of properties of a given type]

  Synopsis           [Given a property type returns the list of properties
  of that type currently located into the property database]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
lsList PropDb_get_props_of_type(const Prop_Type type) 
{
  int i, db_size;
  lsList result = lsCreate();

  nusmv_assert(result != NULL);

  db_size = PropDb_get_size();

  for (i=0; i<db_size; ++i) {
    Prop_ptr p = PropDb_get_prop_at_index(i);

    if (Prop_get_type(p) == type) {
      lsNewEnd(result, (lsGeneric)p, LS_NH);
    }
  }

  return result;
} 


/**Function********************************************************************

  Synopsis           [Add a property to the database from a string and a type]

  Description        [Parses and creates a property of a given type from 
  a string. If the formula is correct, it is added to the 
  property database and its index is returned.
  Otherwise, -1 is returned.
  Valid types are Prop_Ctl, Prop_Ltl, Prop_Psl, Prop_Invar and Prop_Compute.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int PropDb_prop_parse_and_add(Encoding_ptr senc, 
			      const char* str, const Prop_Type type)
{
  char* argv[2];
  int argc = 2;

  nusmv_assert(str != (char*) NULL);
  
  argv[0] = (char*) NULL;
  argv[1] = (char*) str;

  return prop_db_prop_parse_from_arg_and_add(senc, argc, argv, type);
}


/**Function********************************************************************

  Synopsis           [Get a valid property index from a string]

  Description        [Gets the index of a property form a string.
  If the string does not contain a valid index, an error message is emitted
  and -1 is returned.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int PropDb_get_prop_index_from_string(const char* idx)
{
  int idxTemp;
  int db_size = PropDb_get_size();

  if ( db_size <= 0 ) {
    if (cmp_struct_get_flatten_hrc(cmps) == 0) {
      fprintf(nusmv_stderr,
              "The hierarchy must be flattened before. Use the \"flatten_hierarchy\" command.\n");
    }
    else {
      fprintf(nusmv_stderr,"Error: there isn\'t any property available.\n");
    }
    return -1;
  }

  if (util_str2int(idx, &idxTemp) != 0) {
    fprintf(nusmv_stderr, 
            "Error: \"%s\" is not a valid value (must be integer).\n", idx);
    return -1;
  }

  if ( (idxTemp < 0) || (idxTemp >= db_size) ) {
    fprintf(nusmv_stderr, 
            "Error: \"%d\" is not a valid value, must be in the range [0,%d].\n", 
            idxTemp, db_size-1);
    return -1;
  }

  return idxTemp;
}


/**Function********************************************************************

  Synopsis           [Returns the index of the property associated to a trace.]

  Description        [Returns the index of the property associated to a trace.
  -1 if no property is associated to the given trace.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int PropDb_get_prop_index_from_trace_index(const int trace_idx) 
{
  int i;
  int result = -1;
  int db_size = PropDb_get_size();

  for (i=0; i < db_size; ++i) {
    Prop_ptr prop = PropDb_get_prop_at_index(i);

    if (Prop_get_trace(prop) == trace_idx) {
      result = trace_idx;
    }
  }
  
  return result;
}



/**Function********************************************************************
  Synopsis           [Verifies a given property]

  Description        [The DB of properties is searched for a property
  whose unique identifier match the identifier provided and then if
  such a property exists it will be verified calling the appropriate
  model checking algorithm. If the property was checked before, then
  the property is not checked again.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void PropDb_verify_prop_at_index(const int index)
{
  int s = PropDb_get_size();

  if (s < index){
    fprintf(nusmv_stderr, 
	    "Property indexed by %d not present in the database.\n", index);
    fprintf(nusmv_stderr, 
	    "Valid index are in the range [0..%d]\n", s-1);
    nusmv_exit(1);
  }
  else {
    Prop_ptr p = PropDb_get_prop_at_index(index);
    Prop_verify(p);
  }
}


/**Function********************************************************************

  Synopsis           [Verifies all properties of a given type]

  Description        [The DB of properties is searched for a property
  of the given type. All the found properties are then verified
  calling the appropriate model checking algorithm. Properties already
  checked will be ignored.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void PropDb_verify_all_type(Prop_Type type)
{
  int i;
  int s = PropDb_get_size();

  for (i=0; i < s; ++i) {
    Prop_ptr p = PropDb_get_prop_at_index(i);

    if (Prop_get_type(p) == type)  Prop_verify(p);
  }
}


/**Function********************************************************************

  Synopsis           [Verifies all the properties in the DB]

  Description        [All the properties stored in the database not
  yet verified will be verified. The properties are verified following
  the order CTL/COMPUTE/LTL/INVAR.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void PropDb_verify_all()
{
  PropDb_verify_all_type(Prop_Ctl);
  PropDb_verify_all_type(Prop_Compute);
  PropDb_verify_all_type(Prop_Ltl);
  PropDb_verify_all_type(Prop_Psl);
  PropDb_verify_all_type(Prop_Invar);
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Add a property to the database from an arg structure 
  and a type]

  Description        [Parses and creates a property of a given type from 
  an arg structure. If the formula is correct, it is added to the 
  property database and its index is returned.
  Otherwise, -1 is returned.
  Valid types are Prop_Ctl, Prop_Ltl, Prop_Psl, Prop_Invar and Prop_Compute.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int 
prop_db_prop_parse_from_arg_and_add(Encoding_ptr senc, 
				    int argc, char** argv, 
				    const Prop_Type type)
{
  switch (type) {
  case Prop_Ctl:
  case Prop_Ltl:
  case Prop_Psl:
  case Prop_Invar:
  case Prop_Compute:
    /* All ok */
    break;
    
  case Prop_NoType:
    fprintf(nusmv_stderr, "Required to parse a property of unknonw type.\n");
    return -1;
    break;

  default:
    fprintf(nusmv_stderr, "Required to parse a property of unsupported type.\n");
    return -1;
    break;
  } /* switch */ 
  
  {
    node_ptr property;
    node_ptr parsed_command = Nil;

    if (type != Prop_Psl) {
      const char* parsing_type = prop_db_get_prop_type_as_parsing_string(type);
      int parse_result = Parser_ReadCmdFromString(2, argv, 
						  (char*) parsing_type,
						  ";\n", &parsed_command);
      
      if (parse_result != 0 || parsed_command == Nil) {
	fprintf(nusmv_stderr, 
		"Parsing error: expected an \"%s\" expression.\n",
		PropType_to_string(type));
	return -1;
      }
      property = car(parsed_command);
    }
    else {
      int parse_result = Parser_read_psl_from_string(argc, argv, 
						     &parsed_command);
      if (parse_result != 0 || parsed_command == Nil) {
	fprintf(nusmv_stderr, 
		"Parsing error: expected an \"%s\" expression.\n",
		PropType_to_string(type));
	return -1;
      }
      
      property = PslNode_new_context(NULL, parsed_command);
    }

    return PropDb_prop_create_and_add(senc, property, type);
  }
}


/**Function********************************************************************

  Synopsis           [Returns the parsing type given the property type]

  Description        [Returns the parsing type given the property type.
  The returned string must NOT be freed.]

  SideEffects        []

******************************************************************************/
static const char* 
prop_db_get_prop_type_as_parsing_string(const Prop_Type type)
{
  switch (type) {
  case Prop_Ctl: return "CTLWFF ";
  case Prop_Ltl: return "LTLWFF ";
  case Prop_Psl: return "PSLWFF ";
  case Prop_Invar: return "SIMPWFF ";
  case Prop_Compute: return "COMPWFF ";
  }
  
  return "SIMPWFF ";
}
