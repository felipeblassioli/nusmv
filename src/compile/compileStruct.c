/**CFile***********************************************************************

  FileName    [compileStruct.c]

  PackageName [compile]

  Synopsis    [Structure used to store compilation results.]

  Description [Structure used to store compilation results.]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2. 
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

#include "compileInt.h" 

static char rcsid[] UTIL_UNUSED = "$Id: compileStruct.c,v 1.8.4.1.2.1 2005/06/27 14:46:38 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
cmp_struct_ptr cmp_struct_init()
{
  cmp_struct_ptr cmp;
  cmp = ALLOC(cmp_struct_rec, 1);
  cmp->read_model           = 0;
  cmp->flatten_hierarchy    = 0;
  cmp->encode_variables     = 0;
  cmp->process_selector     = 0;
  cmp->build_frames         = 0;
  cmp->compile_check        = 0;
  cmp->build_init           = 0;
  cmp->build_model          = 0;
  cmp->build_flat_model     = 0;
  cmp->build_bool_model     = 0;
  cmp->bmc_setup            = 0;
  cmp->fairness_constraints = 0;
  cmp->coi                  = 0;
  cmp->build_model_setup    = 0;
  cmp->init_expr            = Nil;
  cmp->invar_expr           = Nil;
  cmp->trans_expr           = Nil;
  cmp->procs_expr           = Nil;
  cmp->justice_expr         = Nil;
  cmp->compassion_expr      = Nil;
  cmp->spec_expr            = Nil;
  cmp->compute_expr         = Nil;
  cmp->ltlspec_expr         = Nil;
  cmp->pslspec_expr         = Nil;
  cmp->invar_spec_expr      = Nil;
  return(cmp);
}

int cmp_struct_get_read_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->read_model);
}

void cmp_struct_set_read_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->read_model = 1;
}

int cmp_struct_get_flatten_hrc(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->flatten_hierarchy);
}

void cmp_struct_set_flatten_hrc(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->flatten_hierarchy = 1;
}

int cmp_struct_get_encode_variables(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->encode_variables);
}

void cmp_struct_set_encode_variables(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->encode_variables = 1;
}

int cmp_struct_get_process_selector(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->process_selector);
}

void cmp_struct_set_process_selector(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->process_selector = 1;
}

int cmp_struct_get_build_frames(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_frames);
}

void cmp_struct_set_build_frames(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_frames = 1;
}

int cmp_struct_get_compile_check(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->compile_check);
}

void cmp_struct_set_compile_check(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->compile_check = 1;
}

int cmp_struct_get_build_init(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_init);
}

void cmp_struct_set_build_init(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_init = 1;
}

int cmp_struct_get_build_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_model);
}

void cmp_struct_set_build_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_model = 1;
}

int cmp_struct_get_build_flat_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_flat_model);
}

void cmp_struct_set_build_flat_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_flat_model = 1;
}

int cmp_struct_get_build_bool_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_bool_model);
}

void cmp_struct_set_build_bool_model(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_bool_model = 1;
}

int cmp_struct_get_bmc_setup(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->bmc_setup);
}

void cmp_struct_set_bmc_setup(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->bmc_setup = 1;
}

int cmp_struct_get_fairness(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->fairness_constraints);
}

void cmp_struct_set_fairness(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->fairness_constraints = 1;
}

int cmp_struct_get_coi(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->coi);
}

void cmp_struct_set_coi(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->coi = 1;
}

int cmp_struct_get_build_model_setup(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->build_model_setup);
}

void cmp_struct_set_build_model_setup(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  cmp->build_model_setup = 1;
}

node_ptr cmp_struct_get_init(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->init_expr);
}

void cmp_struct_set_init(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->init_expr = n;
}

node_ptr cmp_struct_get_invar(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->invar_expr);
}

void cmp_struct_set_invar(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->invar_expr = n;
}

node_ptr cmp_struct_get_trans(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->trans_expr);
}
void cmp_struct_set_trans(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->trans_expr = n;
}

node_ptr cmp_struct_get_procs(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->procs_expr);
}
void cmp_struct_set_procs(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->procs_expr = n;
}

node_ptr cmp_struct_get_justice(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->justice_expr);
}
void cmp_struct_set_justice(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->justice_expr = n;
}

node_ptr cmp_struct_get_compassion(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->compassion_expr);
}
void cmp_struct_set_compassion(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->compassion_expr = n;
}

node_ptr cmp_struct_get_spec(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->spec_expr);
}
void cmp_struct_set_spec(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->spec_expr = n;
}

node_ptr cmp_struct_get_compute(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->compute_expr);
}
void cmp_struct_set_compute(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->compute_expr = n;
}

node_ptr cmp_struct_get_ltlspec(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->ltlspec_expr);
}
void cmp_struct_set_ltlspec(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->ltlspec_expr = n;
}

node_ptr cmp_struct_get_pslspec(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->pslspec_expr);
}
void cmp_struct_set_pslspec(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->pslspec_expr = n;
}

node_ptr cmp_struct_get_invar_spec(cmp_struct_ptr cmp)
{
  nusmv_assert(cmp != NULL);
  return(cmp->invar_spec_expr);
}
void cmp_struct_set_invar_spec(cmp_struct_ptr cmp, node_ptr n)
{
  nusmv_assert(cmp != NULL);
  cmp->invar_spec_expr = n;
}

