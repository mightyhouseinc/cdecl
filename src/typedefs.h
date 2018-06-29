/*
**      cdecl -- C gibberish translator
**      src/typedefs.h
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

#ifndef cdecl_typedefs_H
#define cdecl_typedefs_H

/**
 * @file
 * Declares many `typedef` definitions in one file.
 *
 * Some headers are bidirectionally dependent, so `typedef`s were used
 * originally rather than `include`.  However, some old C compilers don't like
 * multiple `typedef` definitions even if the types match.  Hence, just put all
 * `typedef` definitions in one file.
 */

///////////////////////////////////////////////////////////////////////////////

/**
 * Initialization state.
 *
 * We currently only need to discriminate between before and after reading the
 * configuration file, if any, to know whether we should restrict "using"
 * declarations to C++11 and later.  Hence, this could have been a simple bool.
 * However, it was made an enum in the event that discriminating between more
 * initialization states becomes needed in the future.
 */
enum c_init {
  INIT_BEGIN,                           ///< Very beginning of initialization.
  INIT_READ_CONF,                       ///< Read configuration file.
  INIT_DONE                             ///< Initialization done.
};

/**
 * The source location used by Bison.
 */
struct c_loc {
  //
  // These should be either unsigned or size_t, but Bison generates code that
  // tests these for >= 0 which is always true for unsigned types so it
  // generates warnings; hence these are kept as int to eliminate the warnings.
  //
  int first_line;                       ///< First line of location range.
  int first_column;                     ///< First column of location range.
  int last_line;                        ///< Last line of location range.
  int last_column;                      ///< Last column of location range.
};

/**
 * Mode of operation.
 */
enum c_mode {
  MODE_ENGLISH_TO_GIBBERISH,            ///< Convert English into gibberish.
  MODE_GIBBERISH_TO_ENGLISH             ///< Decipher gibberish into English.
};

typedef struct c_ast        c_ast_t;
typedef struct slist_node   c_ast_arg_t;    ///< AST-specific type alias.
typedef unsigned            c_ast_depth_t;  ///< How many `()` deep.
typedef unsigned            c_ast_id_t;     ///< Unique AST node id.
typedef struct c_ast_pair   c_ast_pair_t;
typedef struct c_array      c_array_t;
typedef struct c_block      c_block_t;
typedef enum   c_check      c_check_t;
typedef enum   c_init       c_init_t;
typedef struct c_lang_info  c_lang_info_t;
typedef struct c_loc        c_loc_t;
typedef struct c_builtin    c_builtin_t;
typedef struct c_ecsu       c_ecsu_t;
typedef struct c_func       c_func_t;
typedef struct c_keyword    c_keyword_t;
typedef enum   c_kind       c_kind_t;
typedef enum   c_mode       c_mode_t;
typedef struct c_parent     c_parent_t;
typedef struct c_ptr_mbr    c_ptr_mbr_t;
typedef struct c_ptr_ref    c_ptr_ref_t;
typedef struct c_typedef    c_typedef_t;
typedef enum   v_direction  v_direction_t;

typedef c_loc_t YYLTYPE;                ///< Source location type for Bison.
/// @cond DOXYGEN_IGNORE
#define YYLTYPE_IS_DECLARED       1
#define YYLTYPE_IS_TRIVIAL        1
/// @endcond

///////////////////////////////////////////////////////////////////////////////

#endif /* cdecl_typedefs_H */
/* vim:set et sw=2 ts=2: */
