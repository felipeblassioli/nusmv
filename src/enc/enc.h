/**CHeaderFile*****************************************************************

  FileName    [enc.h]

  PackageName [enc]

  Synopsis    [Public API for the enc package. Basically methods for
  accessing global encodings are provided here]

  Description []

  SeeAlso     [enc.c]

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

#ifndef __ENC_H__
#define __ENC_H__

#include "utils/utils.h"

#include "enc/symb/Encoding.h"
#include "enc/bdd/BddEnc.h"


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure definitions                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Functions declarations                                                    */
/*---------------------------------------------------------------------------*/

EXTERN void Enc_init_symbolic_encoding ARGS((void));
EXTERN void Enc_init_bdd_encoding ARGS((void));
EXTERN void Enc_reinit_bdd_encoding ARGS((void)); 
EXTERN void Enc_quit_encodings ARGS((void));

EXTERN Encoding_ptr Enc_get_symb_encoding ARGS((void));
EXTERN BddEnc_ptr Enc_get_bdd_encoding ARGS((void));



#endif /* __ENC_H__ */
