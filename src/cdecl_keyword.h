/*
**      cdecl -- C gibberish translator
**      src/cdecl_keyword.h
**
**      Copyright (C) 2021-2023  Paul J. Lucas
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

#ifndef cdecl_cdecl_keyword_H
#define cdecl_cdecl_keyword_H

/**
 * @file
 * Declares macros, types, and functions for looking up **cdecl** keyword
 * information.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "c_ast.h"
#include "types.h"
#include "cdecl_parser.h"               /* must go last */

// standard
#include <stdbool.h>

/**
 * @defgroup cdecl-keywords-group Cdecl Keywords
 * Types and functions for **cdecl** keywords.
 * @{
 */

/**
 * Convenience macro for iterating over all **cdecl** keywords.
 *
 * @param VAR The cdecl_keyword loop variable.
 *
 * @sa cdecl_keyword_next()
 */
#define FOREACH_CDECL_KEYWORD(VAR) \
  for ( cdecl_keyword_t const *VAR = NULL; (VAR = cdecl_keyword_next( VAR )) != NULL; )

///////////////////////////////////////////////////////////////////////////////

/**
 * Autocompletion policy for a particular \ref cdecl_keyword.
 */
enum ac_policy {
  /**
   * Default autocompletion policy.
   */
  AC_POLICY_DEFAULT,

  /**
   * Autocomplete only when a keyword is explicitly listed in the \ref
   * cdecl_keyword::ac_next_keywords "ac_next_keywords" of some other keyword.
   *
   * For example, the `bytes` token should be autocompleted only when it
   * follows `aligned`.
   */
  AC_POLICY_IN_NEXT_ONLY,

  /**
   * Autocomplete only if no other keyword matches.
   *
   * For example, the **cdecl** `boolean` keyword is a synonym for either
   * `_Bool` in C or `bool` in C++.  However, `boolean` should _not_ be offered
   * as an autocompletion choice initially since it would be ambiguous with
   * `bool` which is redundant:
   *
   *      cdecl> declare x as bo<tab>
   *      bool boolean
   *
   * Instead, `boolean` should be offered only if the user typed enough as to
   * make it unambiguous (no other keyword matches):
   *
   *      cdecl> declare x as boole<tab>
   */
  AC_POLICY_NO_OTHER
};
typedef enum ac_policy ac_policy_t;

/**
 * **Cdecl** keyword info.
 */
struct cdecl_keyword {
  char const         *literal;          ///< C string literal of the keyword.
  bool                always_find;      ///< Find even when explaining C/C++?

  /**
   * The Bison token (`Y_xxx`), but only if \ref lang_syn is NULL; otherwise 0.
   */
  yytokentype         y_token_id;

  /**
   * Array of language(s)/synonym-keyword pair(s), but only if \ref y_token_id
   * is 0; otherwise NULL.
   *
   * The array is terminated by an element that has #LANG_ANY for \ref
   * c_lang_lit::lang_ids "lang_ids"; hence subset(s) of language(s) cases come
   * first and, failing to match \ref opt_lang against any of those, matches
   * the last (default) element.
   */
  c_lang_lit_t const *lang_syn;

#ifdef WITH_READLINE
  c_lang_id_t         ac_lang_ids;      ///< Language(s) auto-completable in.
  ac_policy_t         ac_policy;        ///< Autocompletion policy.

  /**
   * Pointer to a NULL-terminated array of keywords that should be auto-
   * completed next (after this keyword), if any; otherwise NULL.
   */
  char const *const  *ac_next_keywords;
#endif /* WITH_READLINE */
};

////////// extern functions ///////////////////////////////////////////////////

/**
 * Given a literal, gets the corresponding cdecl_keyword, if any.
 *
 * @param literal The literal to find.
 * @return Returns a pointer to the corresponding cdecl_keyword or NULL if not
 * found.
 */
NODISCARD
cdecl_keyword_t const* cdecl_keyword_find( char const *literal );

/**
 * Iterates to the next **cdecl** keyword.
 *
 * @param k A pointer to the current keyword. For the first iteration, NULL
 * should be passed.
 * @return Returns the next **cdecl** keyword or NULL for none.
 *
 * @note This function isn't normally called directly; use the
 * #FOREACH_CDECL_KEYWORD() macro instead.
 *
 * @sa #FOREACH_CDECL_KEYWORD()
 */
NODISCARD
cdecl_keyword_t const* cdecl_keyword_next( cdecl_keyword_t const *k );

///////////////////////////////////////////////////////////////////////////////

/** @} */

#endif /* cdecl_cdecl_keyword_H */
/* vim:set et sw=2 ts=2: */
