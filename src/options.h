/*
**      cdecl -- C gibberish translator
**      src/options.h
**
**      Copyright (C) 2017-2020  Paul J. Lucas, et al.
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

#ifndef cdecl_options_H
#define cdecl_options_H

/**
 * @file
 * Declares global variables and functions for command-line options.
 */

// local
#include "cdecl.h"                      /* must go first */
#include "c_lang.h"                     /* for c_lang_id_t */
#include "c_type.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <stdbool.h>
#include <stdio.h>                      /* for FILE */

/// @endcond

///////////////////////////////////////////////////////////////////////////////

/**
 * @defgroup cli-options-group Command-Line Options
 * Declares global variables and functions for command-line options.
 * @{
 */

// extern option variables
extern bool         opt_alt_tokens;     ///< Print alternative tokens.
#ifdef ENABLE_CDECL_DEBUG
extern bool         opt_cdecl_debug;    ///< Generate JSON-like debug output?
#endif /* ENABLE_CDECL_DEBUG */
extern char const  *opt_conf_file;      ///< Configuration file path.
extern bool         opt_east_const;     ///< Print in "east const" form?
extern bool         opt_explain;        ///< Assume `explain` if no command?
extern char const  *opt_fin;            ///< File in path.
extern char const  *opt_fout;           ///< File out path.
extern c_graph_t    opt_graph;          ///< Di/Trigraph mode.
extern bool         opt_interactive;    ///< Interactive mode?
extern c_lang_id_t  opt_lang;           ///< Current language.
extern bool         opt_no_conf;        ///< Do not read configuration file.
extern bool         opt_prompt;         ///< Print the prompt?
extern bool         opt_semicolon;      ///< Print `;` at end of gibberish?
extern bool         opt_typedefs;       ///< Load C/C++ standard `typedef`s?

// other extern variables
extern FILE        *fin;                ///< File in.
extern FILE        *fout;               ///< File out.
#ifdef ENABLE_FLEX_DEBUG
extern int          yy_flex_debug;      ///< Flex variable for debugging.
#define opt_flex_debug  yy_flex_debug   ///< Flex debugging.
#endif /* ENABLE_FLEX_DEBUG */
#ifdef YYDEBUG
extern int          yydebug;            ///< Bison variable for debugging.
#define opt_bison_debug yydebug         ///< Bison debugging.
#endif /* YYDEBUG */

////////// extern functions ///////////////////////////////////////////////////

/**
 * Checks if any explicit `int` is set.
 *
 * @return Returns `true` if at least one explicit `int` is set.
 *
 * @sa is_explicit_int()
 */
C_WARN_UNUSED_RESULT
bool any_explicit_int( void );

/**
 * Checks whether \a type_id shall have `int` be printed explicitly for it.
 *
 * @param type_id The integer type to check.
 * @return Returns `true` only if the type given by \a type_id shall have `int`
 * printed explicitly.
 *
 * @sa any_explicit_int()
 */
C_WARN_UNUSED_RESULT
bool is_explicit_int( c_type_id_t type_id );

/**
 * Initializes command-line option variables.
 * On return, `*pargc` and `*pargv` are updated to reflect the remaining
 * command-line with the options removed.
 *
 * @param pargc A pointer to the argument count from `main()`.
 * @param pargv A pointer to the argument values from `main()`.
 */
void options_init( int *pargc, char const ***pargv );

/**
 * Parses the explicit `int` option.
 *
 * @param loc The location of \a s.  If not null and \a s is invalid, calls
 * print_error(); if null and a s is invalid, calls PMESSAGE_EXIT().
 * @param s The null-terminated string to parse.  Valid formats are:
 * @par
 *  Format            | Meaning
 *  ------------------|----------------------------
 *     `i`            | All signed integer types.
 *  `u`               | All unsigned integer types.
 *  [`u`]{`isl`[`l`]} | Possibly `unsigned` `int`, `short`, `long`, or `long long`.
 * @par
 * Multiple formats may be given, one immediately after the other, e.g., `usl`
 * means `unsigned short` and `long`.  Parsing is greedy so commas may be used
 * to separate formats.  For example, `ulll` is parsed as `unsigned long long`
 * and `long` whereas `ul,ll` is parsed as `unsigned long` and `long long`.
 * @par If invalid, an error message is printed to standard error.
 */
void parse_opt_explicit_int( c_loc_t const *loc, char const *s );

/**
 * Prints the string representation of the explicit integer option.
 */
void print_opt_explicit_int( void );

///////////////////////////////////////////////////////////////////////////////

/** @} */

#endif /* cdecl_options_H */
/* vim:set et sw=2 ts=2: */
