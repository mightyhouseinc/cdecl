/*
**      cdecl -- C gibberish translator
**      src/english.c
**
**      Copyright (C) 2017  Paul J. Lucas, et al.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 * Defines functions for printing an AST in pseudo-English.
 */

// local
#include "config.h"                     /* must go first */
#include "c_ast.h"
#include "c_ast_util.h"
#include "c_operator.h"
#include "c_typedef.h"
#include "literals.h"
#include "util.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <assert.h>
#include <stdlib.h>
#include <sysexits.h>

/// @endcond

////////// local functions ////////////////////////////////////////////////////

/**
 * Visitor function that prints \a ast as pseudo-English.
 *
 * @param ast The `c_ast` to print.
 * @param data A pointer to a `FILE` to emit to.
 * @return Always returns `false`.
 */
static bool c_ast_visitor_english( c_ast_t *ast, void *data ) {
  FILE *const eout = REINTERPRET_CAST( FILE*, data );
  bool comma = false;

  switch ( ast->kind ) {
    case K_ARRAY:
      if ( ast->type_id != T_NONE )     // storage class
        FPRINTF( eout, "%s ", c_type_name( ast->type_id ) );
      if ( ast->as.array.size == C_ARRAY_SIZE_VARIABLE )
        FPRINTF( eout, "%s %s ", L_VARIABLE, L_LENGTH );
      FPRINTF( eout, "%s ", L_ARRAY );
      if ( ast->as.array.type_id != T_NONE )
        FPRINTF( eout, "%s ", c_type_name( ast->as.array.type_id ) );
      if ( ast->as.array.size >= 0 )
        FPRINTF( eout, "%d ", ast->as.array.size );
      FPRINTF( eout, "%s ", L_OF );
      break;

    case K_BLOCK:                       // Apple extension
    case K_FUNCTION:
    case K_OPERATOR:
      if ( ast->type_id != T_NONE )     // storage class
        FPRINTF( eout, "%s ", c_type_name( ast->type_id ) );

      switch ( ast->kind ) {
        case K_FUNCTION:
          if ( (ast->type_id & T_MEMBER_ONLY) != T_NONE )
            FPRINTF( eout, "%s ", L_MEMBER );
          break;
        case K_OPERATOR: {
          unsigned const overload_flags = op_get_overload( ast );
          char const *const op_literal =
            overload_flags == OP_MEMBER     ? L_MEMBER      :
            overload_flags == OP_NON_MEMBER ? L_NON_MEMBER  :
            "";
          FPRINTF( eout, "%s%s", SP_AFTER( op_literal ) );
          break;
        }
        default:
          /* suppress warning */;
      } // switch

      FPUTS( c_kind_name( ast->kind ), eout );

      if ( c_ast_args( ast ) != NULL ) {
        FPUTS( " (", eout );            // print function arguments

        for ( c_ast_arg_t const *arg = c_ast_args( ast ); arg != NULL;
              arg = arg->next ) {
          if ( true_or_set( &comma ) )
            FPUTS( ", ", eout );

          c_ast_t const *const arg_ast = C_AST_DATA( arg );
          if ( arg_ast->kind != K_NAME ) {
            //
            // For all kinds except K_NAME, we have to print:
            //
            //      <name> as <english>
            //
            // For K_NAME, e.g.:
            //
            //      void f(x)           // untyped K&R C function argument
            //
            // there's no "as" part, so just let the K_NAME case below print
            // the name itself.
            //
            char const *const name = c_ast_name( arg_ast, V_DOWN );
            if ( name != NULL )
              FPRINTF( eout, "%s %s ", name, L_AS );
            else {
              //
              // If there's no name, it's an unnamed argument, e.g.:
              //
              //    void f(int)
              //
              // so there's no "<name> as" part.
              //
            }
          }

          //
          // For all kinds except K_NAME, we'e already printed the name above,
          // so we have to suppress printing the name again; hence, pass false
          // for print_names here.
          //
          c_ast_t *const nonconst_arg = CONST_CAST( c_ast_t*, arg_ast );
          c_ast_visit( nonconst_arg, V_DOWN, c_ast_visitor_english, data );
        } // for

        FPUTC( ')', eout );
      }

      FPRINTF( eout, " %s ", L_RETURNING );
      break;

    case K_BUILTIN:
      FPUTS( c_type_name( ast->type_id ), eout );
      break;

    case K_ENUM_CLASS_STRUCT_UNION:
      FPRINTF( eout,
        "%s %s",
        c_type_name( ast->type_id ), ast->as.ecsu.ecsu_name
      );
      break;

    case K_NAME:
      if ( ast->name != NULL )
        FPUTS( ast->name, eout );
      break;

    case K_NONE:
      assert( ast->kind != K_NONE );
    case K_PLACEHOLDER:
      assert( ast->kind != K_PLACEHOLDER );

    case K_POINTER:
    case K_REFERENCE:
    case K_RVALUE_REFERENCE: {
      c_type_id_t const qualifier = (ast->type_id & T_MASK_QUALIFIER);
      if ( qualifier != T_NONE )
        FPRINTF( eout, "%s ", c_type_name( qualifier ) );
      FPRINTF( eout, "%s %s ", c_kind_name( ast->kind ), L_TO );
      break;
    }

    case K_POINTER_TO_MEMBER: {
      c_type_id_t const qualifier = (ast->type_id & T_MASK_QUALIFIER);
      if ( qualifier != T_NONE )
        FPRINTF( eout, "%s ", c_type_name( qualifier ) );
      FPRINTF( eout, "%s %s %s %s ", L_POINTER, L_TO, L_MEMBER, L_OF );
      char const *const name = c_type_name( ast->type_id & ~T_MASK_QUALIFIER );
      FPRINTF( eout, "%s%s%s ", SP_AFTER( name ), ast->as.ptr_mbr.class_name );
      break;
    }

    case K_TYPEDEF:
      if ( ast->type_id != T_TYPEDEF_TYPE )
        FPRINTF( eout, "%s ", c_type_name( ast->type_id ) );
      FPUTS( ast->as.c_typedef->ast->name, eout );
      break;

    case K_VARIADIC:
      FPUTS( c_kind_name( ast->kind ), eout );
      break;
  } // switch

  return false;
}

////////// extern functions ///////////////////////////////////////////////////

void c_ast_english( c_ast_t const *ast, FILE *eout ) {
  c_ast_t *const nonconst_ast = CONST_CAST( c_ast_t*, ast );
  c_ast_visit( nonconst_ast, V_DOWN, c_ast_visitor_english, eout );
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
