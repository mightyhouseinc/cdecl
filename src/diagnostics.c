/*
**      cdecl -- C gibberish translator
**      src/diagnostics.c
**
**      Paul J. Lucas
*/

// local
#include "config.h"                     /* must go first */
#include "color.h"
#include "common.h"
#include "diagnostics.h"
#include "lexer.h"
#include "options.h"
#include "prompt.h"
#include "util.h"

// standard
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////

void print_caret( size_t col ) {
  size_t const caret_col = strlen( prompt[0] ) + col;
  PRINT_ERR( "%*s", (int)caret_col, "" );
  SGR_START_COLOR( stderr, caret );
  PUTC_ERR( '^' );
  SGR_END_COLOR( stderr );
  PUTC_ERR( '\n' );
}

void print_error( YYLTYPE const *loc, char const *format, ... ) {
  if ( loc ) {
    print_caret( loc->first_column );
    PRINT_ERR( "%d: ", loc->first_column );
  }
  SGR_START_COLOR( stderr, error );
  PUTS_ERR( "error" );
  SGR_END_COLOR( stderr );
  PUTS_ERR( ": " );

  va_list args;
  va_start( args, format );
  vfprintf( stderr, format, args );
  va_end( args );

  PUTC_ERR( '\n' );
}

void print_hint( char const *format, ... ) {
  PUTS_ERR( "\t(did you mean " );
  va_list args;
  va_start( args, format );
  vfprintf( stderr, format, args );
  va_end( args );
  PUTS_ERR( "?)\n" );
}

void print_warning( YYLTYPE const *loc, char const *format, ... ) {
  if ( loc ) {
    print_caret( loc->first_column );
    PRINT_ERR( "%d: ", loc->first_column );
  }
  SGR_START_COLOR( stderr, warning );
  PUTS_ERR( "warning" );
  SGR_END_COLOR( stderr );
  PUTS_ERR( ": " );

  va_list args;
  va_start( args, format );
  vfprintf( stderr, format, args );
  va_end( args );

  PRINT_ERR( " illegal in %s\n", c_lang_name( opt_lang ) );
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
