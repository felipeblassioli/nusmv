/**CHeaderFile*****************************************************************

  FileName    [bmcUtils.h]

  PackageName [bmc]

  Synopsis    [The public interface to the bmc utilities]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada, Marco Benedetti]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by ITC-irst and University of Trento.

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

  Revision    [$Id: bmcUtils.h,v 1.3.4.2.2.2 2005/03/14 13:26:24 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_UTILS__H
#define _BMC_UTILS__H

#include "trace/Trace.h"

#include "utils/utils.h"
#include "utils/list.h"
#include "utils/ucmd.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define UNKNOWN_OP      -1
#define CONSTANT_EXPR    0
#define LITERAL          1
#define PROP_CONNECTIVE  2
#define TIME_OPERATOR    3

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define isConstantExpr(op) ((op)==TRUEEXP)  || ((op)==FALSEEXP)

#define isVariable(op)     (((op)==DOT) || ((op) == BIT))

#define isPastOp(op)       ((op)==OP_PREC)  || ((op)==OP_NOTPRECNOT) ||    \
                           ((op)==OP_ONCE)  || ((op)==OP_HISTORICAL) ||    \
                           ((op)==SINCE)    || ((op)==TRIGGERED)

#define isBinaryOp(op)     ((op)==AND)      || ((op)==OR)            ||    \
                           ((op)==IFF)      || ((op)==UNTIL)         ||    \
                           ((op)==SINCE)    || ((op)==RELEASES)      ||    \
                           ((op)==TRIGGERED)

#define getOpClass(op) \
  ((op)==TRUEEXP)       || ((op)==FALSEEXP)      ? CONSTANT_EXPR           \
  :                                                                        \
  ((op)==DOT) || ((op) == BIT) || ((op)==NOT)    ? LITERAL                 \
  :                                                                        \
  ((op)==AND)           || ((op)==OR)        ||                            \
  ((op)==IFF)           || ((op)==IF)            ? PROP_CONNECTIVE         \
  :                                                                        \
  ((op)==OP_PREC)       || ((op)==OP_NEXT)   ||                            \
  ((op)==OP_NOTPRECNOT) ||                                                 \
  ((op)==OP_ONCE)       || ((op)==OP_FUTURE) ||                            \
  ((op)==OP_HISTORICAL) || ((op)==OP_GLOBAL) ||                            \
  ((op)==SINCE)         || ((op)==UNTIL)     ||                            \
  ((op)==TRIGGERED)     || ((op)==RELEASES)      ? TIME_OPERATOR           \
  :                                                                        \
                                                   UNKNOWN_OP

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN Trace_ptr 
Bmc_Utils_generate_and_print_cntexample ARGS((BmcVarsMgr_ptr vars_mgr, 
					      SatSolver_ptr solver, 
					      BddEnc_ptr bdd_enc, 
					      be_ptr be_prob, 
					      const int k, 
					      const char* trace_name));

EXTERN boolean Bmc_Utils_IsNoLoopback ARGS((const int l));
EXTERN boolean Bmc_Utils_IsNoLoopbackString ARGS((const char* str));
EXTERN boolean Bmc_Utils_IsSingleLoopback ARGS((const int l));
EXTERN boolean Bmc_Utils_IsAllLoopbacks ARGS((const int l));
EXTERN boolean Bmc_Utils_IsAllLoopbacksString ARGS((const char* str));
EXTERN int Bmc_Utils_GetNoLoopback ARGS((void));
EXTERN int Bmc_Utils_GetAllLoopbacks ARGS((void));
EXTERN const char* Bmc_Utils_GetAllLoopbacksString ARGS((void));

EXTERN int Bmc_Utils_RelLoop2AbsLoop 
ARGS((const int loop, const int k));

EXTERN outcome Bmc_Utils_Check_k_l ARGS((const int k, const int l));

EXTERN int Bmc_Utils_GetSuccTime 
ARGS((const int time, const int k, const int l));

EXTERN int Bmc_Utils_ConvertLoopFromString
ARGS((const char* strValue, outcome* result));

EXTERN void Bmc_Utils_ConvertLoopFromInteger 
ARGS((const int iLoopback, char* szLoopback, const int _bufsize));

EXTERN void
Bmc_Utils_ExpandMacrosInFilename 
ARGS((const char* filename_to_be_expanded,
      const SubstString* table_ptr,
      const size_t table_len,
      char* filename_expanded, size_t buf_len));

/**AutomaticEnd***************************************************************/

#endif /* _BMC_UTILS__H */
