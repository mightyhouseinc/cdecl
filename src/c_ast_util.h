/*
**      cdecl -- C gibberish translator
**      src/c_ast_util.h
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

#ifndef cdecl_c_ast_util_H
#define cdecl_c_ast_util_H

/**
 * @file
 * Declares functions implementing various cdecl-specific algorithms for
 * construcing an Abstract Syntax Tree (AST) for parsed C/C++ declarations.
 */

// local
#include "cdecl.h"                      /* must go first */
#include "types.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * @addtogroup ast-functions-group
 * @{
 */

/**
 * Adds an array to the AST being built.
 *
 * @param ast The AST to append to.
 * @param array_ast The array AST to append.  Its "of" type must be null.
 * @return Returns the AST to be used as the grammar production's return value.
 */
PJL_WARN_UNUSED_RESULT
c_ast_t* c_ast_add_array( c_ast_t *ast, c_ast_t *array_ast );

/**
 * Adds a function-like AST to the AST being built.
 *
 * @param ast The AST to append to.
 * @param ret_ast The AST of the return-type of the function-like AST.
 * @param func_ast The function-like AST to append.  Its "of" type must be
 * null.
 * @return Returns the AST to be used as the grammar production's return value.
 */
PJL_WARN_UNUSED_RESULT
c_ast_t* c_ast_add_func( c_ast_t *ast, c_ast_t *ret_ast, c_ast_t *func_ast );

/**
 * Performs additional checks on an entire AST for semantic errors when
 * casting.
 *
 * @param ast The AST to check.
 * @return Returns `true` only if all checks passed.
 *
 * @sa c_ast_check_declaration()
 */
PJL_WARN_UNUSED_RESULT
bool c_ast_check_cast( c_ast_t const *ast );

/**
 * Checks an entire AST for semantic errors and warnings.
 *
 * @param ast The AST to check.
 * @return Returns `true` only if \a ast error-free.
 *
 * @sa c_ast_check_cast()
 */
PJL_WARN_UNUSED_RESULT
bool c_ast_check_declaration( c_ast_t const *ast );

/**
 * Traverses \a ast attempting to find an AST node having \a kind_ids.
 *
 * @param ast The AST to begin at.
 * @param dir The direction to visit.
 * @param kind_ids The bitwise-or of kind(s) to find.
 * @return Returns a pointer to an AST node having one of \a kind_ids or null
 * if none.
 */
PJL_WARN_UNUSED_RESULT
c_ast_t* c_ast_find_kind_any( c_ast_t *ast, c_visit_dir_t dir,
                              c_kind_id_t kind_ids );

/**
 * Traverses \a ast attempting to find an AST node having a name.
 *
 * @param ast The AST to begin the search at.
 * @param dir The direction to search.
 * @return Returns said name or null if none.
 */
PJL_WARN_UNUSED_RESULT
c_sname_t* c_ast_find_name( c_ast_t const *ast, c_visit_dir_t dir );

/**
 * Traverses \a ast attempting to find an AST node having one of \a types.
 *
 * @param ast The AST to begin at.
 * @param dir The direction to visit.
 * @param types The bitwise-or of type(s) to find.
 * @return Returns a pointer to an AST node having one of \a type_ids or null
 * if none.
 */
PJL_WARN_UNUSED_RESULT
c_ast_t* c_ast_find_type_any( c_ast_t *ast, c_visit_dir_t dir,
                              c_type_t const *types );

/**
 * Checks whether \a ast is an AST for a builtin type.
 *
 * @param ast The AST to check.
 * @param builtin_tid The specific built-in <code>\ref c_type_id_t</code> \a
 * ast can be.  It must be only a base type (no storage classes, qualfiers, or
 * attributes).
 * @return Returns `true` only if the type of \a ast is \a builtin_tid.
 */
PJL_WARN_UNUSED_RESULT
bool c_ast_is_builtin( c_ast_t const *ast, c_type_id_t builtin_tid );

/**
 * Checks whether \a ast is an AST of one of \a kind_ids or a reference or
 * rvalue reference thereto.
 *
 * @param ast The AST to check.
 * @param kind_ids The bitwise-or of the kinds(s) \a ast can be.
 * @return Returns `true` only if \a ast is one of \a kind_ids or a reference
 * or rvalue reference thereto.
 */
PJL_WARN_UNUSED_RESULT
bool c_ast_is_kind_any( c_ast_t const *ast, c_kind_id_t kind_ids );

/**
 * Checks whether \a ast is an AST for a pointer to one of \a tids.
 *
 * @param ast The AST to check.
 * @param tids The bitwise-or of type(s) to check against.
 * @return Returns `true` only if \a ast is a pointer to one of \a tids.
 *
 * @sa c_ast_is_ptr_to_type()
 * @sa c_ast_is_ref_to_tid_any()
 */
PJL_WARN_UNUSED_RESULT
bool c_ast_is_ptr_to_tid_any( c_ast_t const *ast, c_type_id_t tids );

/**
 * Checks whether \a ast is an AST for a pointer to a certain type.
 * For example:
 *  + `c_ast_is_ptr_to_type( ast, &T_ANY, &C_TYPE_LIT_B(TB_CHAR) )`
 *    returns `true` only if \a ast is pointer to `char` (`char*`) _exactly_.
 *  + `c_ast_is_ptr_to_type( ast, &T_ANY, &C_TYPE_LIT(TB_CHAR, TS_CONST, TA_NONE) )`
 *    returns `true` only if \a ast is a pointer to `const` `char` (`char
 *    const*`) _exactly_.
 *  + `c_ast_is_ptr_to_type( ast, &C_TYPE_LIT_S(c_type_id_compl( TS_CONST )), &C_TYPE_LIT_B(TB_CHAR) )`
 *    returns `true` if \a ast is a pointer to `char` regardless of `const`
 *    (`char*` or `char const*`).
 *
 * @param ast The AST to check.
 * @param mask_type The type mask to apply to the type of \a ast before
 * equality comparison with \a equal_type.
 * @param equal_type The _exact_ type to check against.
 * @return Returns `true` only if \a ast (masked with \a mask_type) is a
 * pointer to \a equal_type.
 *
 * @sa c_ast_is_ptr_to_tid_any()
 */
PJL_WARN_UNUSED_RESULT
bool c_ast_is_ptr_to_type( c_ast_t const *ast, c_type_t const *mask_type,
                           c_type_t const *equal_type );

/**
 * Checks whether \a ast is an AST for a reference or rvalue reference to one
 * of \a tids.
 *
 * @param ast The AST to check.
 * @param tids The bitwise-or of type(s) to check against.
 * @return Returns `true` only if \a ast is a reference or rvalue reference to
 * one of \a tids.
 *
 * @sa c_ast_is_ptr_to_tid_any()
 */
PJL_WARN_UNUSED_RESULT
bool c_ast_is_ref_to_tid_any( c_ast_t const *ast, c_type_id_t tids );

/**
 * "Patches" \a type_ast into \a decl_ast only if:
 *  + \a type_ast has no parent.
 *  + The depth of \a type_ast is less than that of \a decl_ast.
 *  + \a decl_ast still contains an AST node of type
 *    <code>\ref K_PLACEHOLDER</code>.
 *
 * @param type_ast The AST of the initial type.
 * @param decl_ast The AST of a declaration.  May be null.
 * @return Returns the final AST.
 */
PJL_WARN_UNUSED_RESULT
c_ast_t* c_ast_patch_placeholder( c_ast_t *type_ast, c_ast_t *decl_ast );

/**
 * Takes the name, if any, away from \a ast
 * (with the intent of giving it to another AST).
 *
 * @param ast The AST (or one of its child nodes) to take from.
 * @return Returns said name or en empty name.
 */
PJL_WARN_UNUSED_RESULT
c_sname_t c_ast_take_name( c_ast_t *ast );

/**
 * Checks \a ast to see if it contains one or more of \a type.
 * If so, removes them.
 * This is used in cases like:
 * @code
 *  explain typedef int *p
 * @endcode
 * that should be explained as:
 * @code
 *  declare p as type pointer to int
 * @endcode
 * and _not_:
 * @code
 *  declare p as pointer to typedef int
 * @endcode
 *
 * @param ast The AST to check.
 * @param type The bitwise-or of type(s) to find.
 * @return Returns the taken type.
 */
PJL_WARN_UNUSED_RESULT
c_type_t c_ast_take_type_any( c_ast_t *ast, c_type_t const *type );

/**
 * Un-pointers \a ast, i.e., if \a ast is a <code>\ref K_POINTER</code>,
 * returns the pointed-to AST.
 *
 * @note `typedef`s are stripped.
 * @note Even though pointers are "dereferenced," this function isn't called
 * `c_ast_dereference` to eliminate confusion with C++ references.
 *
 * @param ast The AST to un-pointer.
 * @return Returns the pointed-to AST or null if \a ast is not a pointer.
 *
 * @sa c_ast_unreference()
 * @sa c_ast_untypedef()
 */
PJL_WARN_UNUSED_RESULT
c_ast_t const* c_ast_unpointer( c_ast_t const *ast );

/**
 * Un-references \a ast, i.e., if \a ast is a <code>\ref K_REFERENCE</code>
 * returns the referenced AST.
 *
 * @note `typedef`s are stripped.
 * @note Only <code>\ref K_REFERENCE</code> is un-referenced, not
 * <code>\ref K_RVALUE_REFERENCE</code>.
 *
 * @param ast The AST to un-reference.
 * @return Returns the referenced AST or \a ast if \a ast is not a reference.
 *
 * @sa c_ast_unpointer()
 * @sa c_ast_untypedef()
 */
PJL_WARN_UNUSED_RESULT
c_ast_t const* c_ast_unreference( c_ast_t const *ast );

/**
 * Un-typedefs \a ast, i.e., if \a ast is a <code>\ref K_TYPEDEF</code>,
 * returns the underlying type AST.
 *
 * @param ast The AST to un-typedef.
 * @return Returns the underlying type AST or \a ast if \a ast is not a
 * `typedef`.
 *
 * @sa c_ast_unpointer()
 * @sa c_ast_unreference()
 */
PJL_WARN_UNUSED_RESULT
c_ast_t const* c_ast_untypedef( c_ast_t const *ast );

///////////////////////////////////////////////////////////////////////////////

/** @} */

#endif /* cdecl_c_ast_util_H */
/* vim:set et sw=2 ts=2: */
