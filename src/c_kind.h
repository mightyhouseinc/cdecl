/*
**      cdecl -- C gibberish translator
**      src/c_kind.h
**
**      Copyright (C) 2017-2022  Paul J. Lucas
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

#ifndef cdecl_c_kind_H
#define cdecl_c_kind_H

/**
 * @file
 * Declares types and functions for kinds of AST nodes in C/C++ declarations.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "types.h"

/**
 * @defgroup c-kinds-group C/C++ Declaration Kinds
 * Types and functions for kinds of AST nodes in C/C++ declarations.
 * @{
 */

/**
 * Shorthand for use inside a `switch` statement on an AST's kind to:
 *
 * 1. Assert that it's not #K_PLACEHOLDER because it shouldn't occur in a
 *    completed AST.
 *
 * 2. Suppress the warning that otherwise would be given because #K_PLACEHOLDER
 *    is not handled by the `switch`.
 *
 * For example:
 *
 *      switch ( ast->kind ) {
 *        // ...
 *        CASE_K_PLACEHOLDER;
 *      }
 */
#define CASE_K_PLACEHOLDER  \
  case K_PLACEHOLDER:       \
    assert( 0 );            \
    break

///////////////////////////////////////////////////////////////////////////////

/**
 * Kinds of AST nodes comprising a C/C++ declaration.
 *
 * A given AST node may only have a single kind and _not_ be a bitwise-or of
 * kinds.  However, a bitwise-or of kinds may be used to test whether a given
 * AST node is any _one_ of those kinds.
 */
enum c_ast_kind {
  /**
   * Temporary node in AST.  This is needed in two cases:
   *
   * 1. Array declarations or casts.  Consider:
   *
   *         int a[2][3]
   *
   *    At the first `[`, we know it's an _array 2 of [something of]*_ `int`,
   *    but we don't yet know either what the "something" is or whether it will
   *    turn out to be nothing.  It's not until the second `[` that we know
   *    it's an _array 2 of array 3 of_ `int`.  (Had the `[3]` not been there,
   *    then it would have been just _array 2 of_ `int`.)
   *
   * 2. Nested declarations or casts (inside parentheses).  Consider:
   *
   *         int (*a)[2]
   *
   *    At the `*`, we know it's a _pointer to [something of]*_ `int`, but,
   *    similar to the array case, we don't yet know either what the
   *    "something" is or whether it will turn out to be nothing.  It's not
   *    until the `[` that we know it's a _pointer to array 2 of_ `int`.  (Had
   *    the `[2]` not been there, then it would have been just _pointer to_
   *    `int` (with unnecessary parentheses).
   *
   * In either case, a placeholder node is created to hold the place of the
   * "something" in the AST.
   */
  K_PLACEHOLDER             = (1u << 0),

  /**
   * Built-in type, e.g., `void`, `char`, `int`, etc.
   */
  K_BUILTIN                 = (1u << 1),

  /**
   * A `class,` `struct,` or `union`.
   */
  K_CLASS_STRUCT_UNION      = (1u << 2),

  /**
   * Name only.  It's used as the initial kind for an identifier ("name") until
   * we know its actual type (if ever).  However, it's also used for pre-
   * prototype typeless function parameters in K&R C, e.g., `double sin(x)`.
   */
  K_NAME                    = (1u << 3),

  /**
   * `typedef` type, e.g., `size_t`.
   */
  K_TYPEDEF                 = (1u << 4),

  /**
   * Variadic (`...`) function parameter.
   */
  K_VARIADIC                = (1u << 5),

  ////////// "parent" kinds ///////////////////////////////////////////////////

  /**
   * Array.
   */
  K_ARRAY                   = (1u << 6),

  /**
   * An `enum`.
   *
   * @note This is a "parent" kind because `enum` in C++11 and later can be
   * "of" a fixed type.
   */
  K_ENUM                    = (1u << 7),

  /**
   * C or C++ pointer.
   */
  K_POINTER                 = (1u << 8),

  /**
   * C++ pointer-to-member.
   */
  K_POINTER_TO_MEMBER       = (1u << 9),

  /**
   * C++ reference.
   */
  K_REFERENCE               = (1u << 10),

  /**
   * C++11 rvalue reference.
   */
  K_RVALUE_REFERENCE        = (1u << 11),

  ////////// function-like "parent" kinds /////////////////////////////////////

  /**
   * C++ constructor.
   */
  K_CONSTRUCTOR             = (1u << 12),

  /**
   * C++ destructor.
   */
  K_DESTRUCTOR              = (1u << 13),

  ////////// function-like "parent" kinds that have return values /////////////

  /**
   * Block (Apple extension).
   *
   * @sa [Apple's Extensions to C](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1370.pdf)
   * @sa [Blocks Programming Topics](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Blocks)
   */
  K_APPLE_BLOCK             = (1u << 14),

  /**
   * Function.
   */
  K_FUNCTION                = (1u << 15),

  /**
   * C++ overloaded operator.
   */
  K_OPERATOR                = (1u << 16),

  /**
   * C++ user-defined conversion operator.
   */
  K_USER_DEF_CONVERSION     = (1u << 17),

  /**
   * C++11 user-defined literal.
   */
  K_USER_DEF_LITERAL        = (1u << 18),
};

/**
 * Shorthand for either #K_ENUM or #K_CLASS_STRUCT_UNION.
 */
#define K_ANY_ENUM_CLASS_STRUCT_UNION \
                                  ( K_ENUM | K_CLASS_STRUCT_UNION )

/**
 * Shorthand for any kind of function-like AST: #K_APPLE_BLOCK, #K_CONSTRUCTOR,
 * #K_DESTRUCTOR, #K_FUNCTION, #K_OPERATOR, #K_USER_DEF_CONVERSION, or
 * #K_USER_DEF_LITERAL.
 */
#define K_ANY_FUNCTION_LIKE       ( K_APPLE_BLOCK | K_CONSTRUCTOR \
                                  | K_DESTRUCTOR | K_FUNCTION | K_OPERATOR \
                                  | K_USER_DEF_CONVERSION | K_USER_DEF_LITERAL )

/**
 * Shorthand for any kind of "object" that can be the type of a variable or
 * constant, i.e., something to which `sizeof` can be applied: #K_ARRAY,
 * #K_BUILTIN, #K_CLASS_STRUCT_UNION, #K_ENUM, #K_POINTER,
 * #K_POINTER_TO_MEMBER, #K_REFERENCE, #K_RVALUE_REFERENCE, or #K_TYPEDEF.
 */
#define K_ANY_OBJECT              ( K_ANY_POINTER | K_ANY_REFERENCE | K_ARRAY \
                                  | K_BUILTIN | K_ANY_ENUM_CLASS_STRUCT_UNION \
                                  | K_TYPEDEF )

/**
 * Shorthand for any kind of parent: #K_APPLE_BLOCK, #K_ARRAY, #K_CONSTRUCTOR,
 * #K_DESTRUCTOR, #K_ENUM, #K_FUNCTION, #K_OPERATOR, #K_POINTER,
 * #K_POINTER_TO_MEMBER, #K_REFERENCE, #K_RVALUE_REFERENCE,
 * #K_USER_DEF_CONVERSION, or #K_USER_DEF_LITERAL.
 *
 * @note #K_TYPEDEF is intentionally _not_ included.
 *
 * @sa #K_ANY_REFERRER
 */
#define K_ANY_PARENT              ( K_ANY_FUNCTION_LIKE | K_ANY_POINTER \
                                  | K_ANY_REFERENCE | K_ARRAY | K_ENUM )

/**
 * Shorthand for any kind of pointer: #K_POINTER or #K_POINTER_TO_MEMBER.
 */
#define K_ANY_POINTER             ( K_POINTER | K_POINTER_TO_MEMBER )

/**
 * Shorthand for any kind of reference: #K_REFERENCE or #K_RVALUE_REFERENCE.
 */
#define K_ANY_REFERENCE           ( K_REFERENCE | K_RVALUE_REFERENCE )

/**
 * Shorthand for any kind that has a pointer to another AST: #K_ANY_PARENT or
 * #K_TYPEDEF.
 *
 * @sa #K_ANY_PARENT
 */
#define K_ANY_REFERRER            ( K_ANY_PARENT | K_TYPEDEF )

////////// extern functions ///////////////////////////////////////////////////

/**
 * Gets the name of \a kind.
 *
 * @param kind The \ref c_ast_kind to get the name for.
 * @return Returns said name.
 */
PJL_WARN_UNUSED_RESULT
char const* c_kind_name( c_ast_kind_t kind );

///////////////////////////////////////////////////////////////////////////////

/** @} */

#endif /* cdecl_c_kind_H */
/* vim:set et sw=2 ts=2: */
