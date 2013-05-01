/**CFile*****************************************************************

  FileName    [enc.c]

  PackageName [enc]

  Synopsis    [Package level code of package enc]

  Description []

  SeeAlso     [enc.h, encInt.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc'' package of NuSMV version 2. 
  Copyright (C) 2003 by ITC-irst.

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

#include "enc.h"
#include "encInt.h"


/**Variable********************************************************************

  Synopsis    [Global symbolic encoding]

  Description [This instance must be accessed via dedicated methods]

  SeeAlso     []

******************************************************************************/
static Encoding_ptr global_symb_enc = ENCODING(NULL);


/**Variable********************************************************************

  Synopsis    [Global encoding for BDDs]

  Description [This instance must be accessed via dedicated methods]

  SeeAlso     []

******************************************************************************/
static BddEnc_ptr global_bdd_enc = BDD_ENC(NULL);


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void enc_set_symb_encoding ARGS((Encoding_ptr senc));
static void enc_set_bdd_encoding ARGS((BddEnc_ptr enc));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the symbolic encoding for this session]

  Description        [Call it to initialize for the current session the
  encoding, before flattening. In the current implementation, you must
  call this *before* the flattening phase. After the flattening, 
  you must initialize the bdd encoding as well, and after you created the
  boolean sexp fsm, you must reinitialize the bdd encodings by calling
  Enc_reinit_bdd_encoding. Don't forget to call Enc_quit_encodings when
  the session ends. ]

  SideEffects        []

  SeeAlso            [Enc_init_bdd_encoding, Enc_reinit_bdd_encoding,
  Enc_quit_encodings]
******************************************************************************/
void Enc_init_symbolic_encoding()
{
  nusmv_assert(global_symb_enc == ENCODING(NULL));

  if (opt_verbose_level_gt(options, 3)) {
      fprintf(nusmv_stdout, "\nInitializing global symbolic encoding...\n");
  }

  global_symb_enc = Encoding_create();

  if (opt_verbose_level_gt(options, 3)) {
      fprintf(nusmv_stdout, "Global symbolic encoding initialized.\n");
  }
}


/**Function********************************************************************

  Synopsis           [Initializes the bdd enc for this session]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Enc_init_bdd_encoding()
{
  ENCODING_CHECK_INSTANCE(global_symb_enc);
  nusmv_assert(global_bdd_enc == BDD_ENC(NULL));

  if (opt_verbose_level_gt(options, 3)) {
      fprintf(nusmv_stdout, "\nInitializing global BDD encoding...\n");
  }

  global_bdd_enc = BddEnc_create(global_symb_enc, dd_manager);  

  if (opt_verbose_level_gt(options, 3)) {
      fprintf(nusmv_stdout, "Global BDD encoding firstly initialized.\n");
  }
}


/**Function********************************************************************

  Synopsis           [Reinitialize the BddEnc instance, getting new variable 
  added by booleanization of scalar sxp fsm of the symbolic encoding]

  Description        [The reinitialization is due to the current
  implementation, and might be no longer required in the future.  Call
  after you created the boolean sexp fsm.  The current global BddEnc
  instance will be destroyed, and substituted by a new instance. Any 
  previous reference to the old encoding won't be no longer usable.]

  SideEffects        [Current global BddEnc instance is substituted by a 
  new instance]

  SeeAlso            []
******************************************************************************/
void Enc_reinit_bdd_encoding() 
{
  ENCODING_CHECK_INSTANCE(global_symb_enc);
  BDD_ENC_CHECK_INSTANCE(global_bdd_enc);

  if (opt_verbose_level_gt(options, 3)) {
      fprintf(nusmv_stdout, "\nRe-initializing BDD encoding...\n");
  }

  /* this will substitute the current BddEnc instance */
  enc_set_bdd_encoding(BddEnc_create(global_symb_enc, dd_manager));

  if (opt_verbose_level_gt(options, 3)) {
    fprintf(nusmv_stdout, "BDD encoding re-initialized\n");
  }
}


/**Function********************************************************************

  Synopsis           [Call to destroy encodings, when session ends]

  Description        [Call to destroy encodings, when session ends. 
  Enc_init_encodings had to be called before calling this function.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Enc_quit_encodings()
{
  enc_set_symb_encoding(ENCODING(NULL));
  enc_set_bdd_encoding(BDD_ENC(NULL));
  encoding_quit_boolean_type();
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
Encoding_ptr Enc_get_symb_encoding(void)
{
  ENCODING_CHECK_INSTANCE(global_symb_enc);
  return global_symb_enc;
}
 

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
BddEnc_ptr Enc_get_bdd_encoding(void)
{
  BDD_ENC_CHECK_INSTANCE(global_bdd_enc);
  return global_bdd_enc;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Set the global symbolic encoding]

  Description        [Set the global symbolic encoding. If it was already 
  set, it is detroyed before the assignment]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
static void enc_set_symb_encoding(Encoding_ptr senc)
{
  if (global_symb_enc != ENCODING(NULL)) {
    Encoding_destroy(global_symb_enc);
  }
      
  global_symb_enc = senc;      
}


/**Function********************************************************************

  Synopsis           [Set the global bdd encoding]

  Description        [Set the global bdd encoding. If it was already 
  set, it is detroyed before the assignment]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
static void enc_set_bdd_encoding(BddEnc_ptr enc)
{
  if (global_bdd_enc != BDD_ENC(NULL)) {
    BddEnc_destroy(global_bdd_enc);
  }
      
  global_bdd_enc = enc;      
}
