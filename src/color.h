/*
**      cdecl -- C gibberish translator
**      src/color.h
*/

#ifndef cdecl_color_H
#define cdecl_color_H

/**
 * @file
 * Contains constants, macros, types, global variables, and functions for
 * printing in color.
 */

// local
#include "config.h"                     /* must go first */

// standard
#include <stdbool.h>

///////////////////////////////////////////////////////////////////////////////

#define SGR_BG_BLACK        "40"
#define SGR_BG_RED          "41"
#define SGR_BG_GREEN        "42"
#define SGR_BG_YELLOW       "43"
#define SGR_BG_BLUE         "44"
#define SGR_BG_MAGENTA      "45"
#define SGR_BG_CYAN         "46"
#define SGR_BG_WHITE        "47"

#define SGR_FG_BLACK        "30"
#define SGR_FG_RED          "31"
#define SGR_FG_GREEN        "32"
#define SGR_FG_YELLOW       "33"
#define SGR_FG_BLUE         "34"
#define SGR_FG_MAGENTA      "35"
#define SGR_FG_CYAN         "36"
#define SGR_FG_WHITE        "37"

#define SGR_BOLD            "1"
#define SGR_CAP_SEP         ":"         /* capability separator */
#define SGR_SEP             ";"         /* attribute/RGB separator */

// gray is acyually "bold black"
#define SGR_BG_GRAY         SGR_BG_BLACK SGR_SEP SGR_BOLD
#define SGR_FG_GRAY         SGR_FG_BLACK SGR_SEP SGR_BOLD

#define SGR_START           "\33[%sm"   /* start color sequence */
#define SGR_END             "\33[m"     /* end color sequence */
#define SGR_EL              "\33[K"     /* Erase in Line (EL) sequence */

#define COLOR_WHEN_DEFAULT  COLOR_NOT_FILE
#define COLORS_DEFAULT                                        \
  "caret="        SGR_FG_GREEN  SGR_SEP SGR_BOLD  SGR_CAP_SEP \
  "error="        SGR_FG_RED    SGR_SEP SGR_BOLD  SGR_CAP_SEP \
  "HELP-keyword="                       SGR_BOLD  SGR_CAP_SEP \
  "HELP-nonterm=" SGR_FG_CYAN                     SGR_CAP_SEP \
  "HELP-punct="   SGR_FG_GRAY                     SGR_CAP_SEP \
  "HELP-title="   SGR_FG_BLUE   SGR_SEP SGR_BOLD  SGR_CAP_SEP \
  "locus="                              SGR_BOLD  SGR_CAP_SEP \
  "note="         SGR_FG_CYAN   SGR_SEP SGR_BOLD  SGR_CAP_SEP \
  "warning="      SGR_FG_YELLOW SGR_SEP SGR_BOLD  SGR_CAP_SEP

/**
 * Starts printing in the given, predefined color.
 *
 * @param STREAM The FILE to use.
 * @param COLOR The predefined color.
 * @hideinitializer
 */
#define SGR_START_COLOR(STREAM,COLOR) BLOCK(            \
  if ( colorize && (sgr_ ## COLOR) )                    \
    FPRINTF( (STREAM), SGR_START SGR_EL, (sgr_ ## COLOR) ); )

/**
 * Ends printing in color.
 *
 * @param STREAM The FILE to use.
 * @hideinitializer
 */
#define SGR_END_COLOR(STREAM) \
  BLOCK( if ( colorize ) FPUTS( SGR_END SGR_EL, (STREAM) ); )

/**
 * When to colorize output.
 */
enum color_when {
  COLOR_NEVER,                          // never colorize
  COLOR_ISATTY,                         // colorize only if isatty(3)
  COLOR_NOT_FILE,                       // colorize only if !ISREG stdout
  COLOR_ALWAYS                          // always colorize
};
typedef enum color_when color_when_t;

// extern variables
extern bool         colorize;           // colorize diagnostics?
extern char const  *sgr_caret;
extern char const  *sgr_error;
extern char const  *sgr_help_keyword;
extern char const  *sgr_help_nonterm;
extern char const  *sgr_help_punct;
extern char const  *sgr_help_title;
extern char const  *sgr_warning;

///////////////////////////////////////////////////////////////////////////////

/**
 * Parses and sets the sequence of gcc color capabilities.
 *
 * @param capabilities The gcc capabilities to parse.
 * @return Returns \c true only if at least one capability was parsed
 * successfully.
 */
bool parse_gcc_colors( char const *capabilities );

/**
 * Determines whether we should emit escape sequences for color.
 *
 * @param c The color_when value.
 * @return Returns \c true only if we should do color.
 */
bool should_colorize( color_when_t c );

///////////////////////////////////////////////////////////////////////////////

#endif /* cdecl_color_H */
/* vim:set et sw=2 ts=2: */
