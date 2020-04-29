/*
**      cdecl -- C gibberish translator
**      src/c_type.h
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

#ifndef cdecl_c_type_H
#define cdecl_c_type_H

/**
 * @file
 * Declares constants, types, and functions for C/C++ types.
 */

// local
#include "cdecl.h"                      /* must go first */
#include "c_lang.h"                     /* for c_lang_id_t */
#include "typedefs.h"                   /* for c_loc_t */
#include "util.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <stdbool.h>
#include <inttypes.h>                   /* for PRIX64, etc. */
#include <stdlib.h>                     /* for free(3) */

_GL_INLINE_HEADER_BEGIN
#ifndef C_TYPE_INLINE
# define C_TYPE_INLINE _GL_INLINE
#endif /* C_TYPE_INLINE */

/// @endcond

/**
 * @defgroup c-types-group C/C++ Types
 * Constants, types, and functions for C/C++ types.
 * @{
 */

////////// types //////////////////////////////////////////////////////////////

// types
#define T_NONE                0ull                  /**< No type.             */
#define T_VOID                0x0000000000000001ull /**< `void`               */
#define T_AUTO_TYPE           0x0000000000000002ull /**< C++11's `auto`.      */
#define T_BOOL                0x0000000000000004ull /**< `_Bool` or `bool`    */
#define T_CHAR                0x0000000000000008ull /**< `char`               */
#define T_CHAR8_T             0x0000000000000010ull /**< `char8_t`            */
#define T_CHAR16_T            0x0000000000000020ull /**< `char16_t`           */
#define T_CHAR32_T            0x0000000000000040ull /**< `char32_t`           */
#define T_WCHAR_T             0x0000000000000080ull /**< `wchar_t`            */
#define T_SHORT               0x0000000000000100ull /**< `short`              */
#define T_INT                 0x0000000000000200ull /**< `int`                */
#define T_LONG                0x0000000000000400ull /**< `long`               */
#define T_LONG_LONG           0x0000000000000800ull /**< `long long`          */
#define T_SIGNED              0x0000000000001000ull /**< `signed`             */
#define T_UNSIGNED            0x0000000000002000ull /**< `unsigned`           */
#define T_FLOAT               0x0000000000004000ull /**< `float`              */
#define T_DOUBLE              0x0000000000008000ull /**< `double`             */
#define T_COMPLEX             0x0000000000010000ull /**< `_Complex`           */
#define T_IMAGINARY           0x0000000000020000ull /**< `_Imaginary`         */
#define T_ENUM                0x0000000000040000ull /**< `enum`               */
#define T_STRUCT              0x0000000000080000ull /**< `struct`             */
#define T_UNION               0x0000000000100000ull /**< `union`              */
#define T_CLASS               0x0000000000200000ull /**< `class`              */
#define T_NAMESPACE           0x0000000000400000ull /**< `namespace`          */
#define T_SCOPE               0x0000000000800000ull /**< Generic scope.       */
#define T_TYPEDEF_TYPE        0x0000000001000000ull /**< E.g., `size_t`       */

// storage classes
#define T_AUTO_STORAGE        0x0000000010000000ull /**< C's `auto`.          */
#define T_APPLE_BLOCK         0x0000000020000000ull /**< Block.               */
#define T_EXTERN              0x0000000040000000ull /**< `extern`             */
#define T_MUTABLE             0x0000000080000000ull /**< `mutable`            */
#define T_REGISTER            0x0000000100000000ull /**< `register`           */
#define T_STATIC              0x0000000200000000ull /**< `static`             */
#define T_THREAD_LOCAL        0x0000000400000000ull /**< `thread_local`       */
#define T_TYPEDEF             0x0000000800000000ull /**< `typedef`            */

// storage-class-like
#define T_CONSTEVAL           0x0000001000000000ull /**< `consteval`          */
#define T_CONSTEXPR           0x0000002000000000ull /**< `constexpr`          */
#define T_DEFAULT             0x0000004000000000ull /**< `= default`          */
#define T_DELETE              0x0000008000000000ull /**< `= delete`           */
#define T_EXPLICIT            0x0000010000000000ull /**< `explicit`           */
#define T_FINAL               0x0000020000000000ull /**< `final`              */
#define T_FRIEND              0x0000040000000000ull /**< `friend`             */
#define T_INLINE              0x0000080000000000ull /**< `inline`             */
#define T_NOEXCEPT            0x0000100000000000ull /**< `noexcept`           */
#define T_OVERRIDE            0x0000200000000000ull /**< `override`           */
#define T_PURE_VIRTUAL        0x0000400000000000ull /**< `= 0`                */
#define T_THROW               0x0000800000000000ull /**< `throw()`            */
#define T_VIRTUAL             0x0001000000000000ull /**< `virtual`            */

// attributes
#define T_CARRIES_DEPENDENCY  0x0002000000000000ull /**< `carries_dependency` */
#define T_DEPRECATED          0x0004000000000000ull /**< `deprecated`         */
#define T_MAYBE_UNUSED        0x0008000000000000ull /**< `maybe_unused`       */
#define T_NODISCARD           0x0010000000000000ull /**< `nodiscard`          */
#define T_NORETURN            0x0020000000000000ull /**< `noreturn`           */

// qualifiers
#define T_ATOMIC              0x0100000000000000ull /**< `_Atomic`            */
#define T_CONST               0x0200000000000000ull /**< `const`              */
#define T_RESTRICT            0x0400000000000000ull /**< `restrict`           */
#define T_VOLATILE            0x0800000000000000ull /**< `volatile`           */

// ref-qualifiers
#define T_REFERENCE           0x1000000000000000ull /**< `void f() &`         */
#define T_RVALUE_REFERENCE    0x2000000000000000ull /**< `void f() &&`        */

// bit masks
#define T_MASK_TYPE           0x000000000FFFFFFFull /**< Type bitmask.        */
#define T_MASK_STORAGE        0x0001FFFFF0000000ull /**< Storage bitmask.     */
#define T_MASK_ATTRIBUTE      0x003E000000000000ull /**< Attribute bitmask.   */
#define T_MASK_QUALIFIER      0x0F00000000000000ull /**< Qualifier bitmask.   */
#define T_MASK_REF_QUALIFIER  0xF000000000000000ull /**< Ref-qual bitmask.    */

// shorthands

/** Shorthand for any character type. */
#define T_ANY_CHAR            ( T_CHAR | T_WCHAR_T \
                              | T_CHAR8_T | T_CHAR16_T | T_CHAR32_T )

/** Shorthand for any floating-point type. */
#define T_ANY_FLOAT           ( T_FLOAT | T_DOUBLE )

/** Shorthand for any reference type. */
#define T_ANY_REFERENCE       ( T_REFERENCE | T_RVALUE_REFERENCE )

/** Shorthand for `class`, `struct`, or `union`. */
#define T_CLASS_STRUCT_UNION  ( T_CLASS | T_STRUCT | T_UNION )

/** The only type(s) that can be applied to constructors. */
#define T_CONSTRUCTOR         ( T_CONSTEXPR | T_EXPLICIT | T_FRIEND | T_INLINE \
                              | T_NOEXCEPT | T_THROW )

/** The type(s) that can apply only to constructors. */
#define T_CONSTRUCTOR_ONLY    T_EXPLICIT

/** Shorthand for an `int` modifier. */
#define T_INT_MODIFIER        ( T_SHORT | T_LONG | T_LONG_LONG | T_SIGNED \
                              | T_UNSIGNED )

/** The type(s) that can apply only to members. */
#define T_MEMBER_ONLY         ( T_CONST | T_VOLATILE | T_DEFAULT | T_DELETE \
                              | T_OVERRIDE | T_FINAL | T_VIRTUAL \
                              | T_REFERENCE | T_RESTRICT | T_RVALUE_REFERENCE )

/** The type(s) that can apply only to non-members. */
#define T_NON_MEMBER_ONLY     T_FRIEND

/** The type(s) that can apply to user-defined conversion operators. */
#define T_USER_DEF_CONV       ( T_CONST | T_CONSTEXPR | T_EXPLICIT | T_FINAL  \
                              | T_FRIEND | T_INLINE | T_NOEXCEPT | T_OVERRIDE \
                              | T_THROW | T_PURE_VIRTUAL | T_VIRTUAL)

/**
 * Hexadecimal print conversion specifier for <code>\ref c_type_id_t</code>.
 */
#define PRIX_C_TYPE_T         PRIX64

////////// inline functions ///////////////////////////////////////////////////

/**
 * Frees the data for a `c_type_id_t`.
 * @note For platforms with 64-bit pointers, this is a no-op.
 *
 * @param data The data to free.
 *
 * @sa c_type_id_data_new(c_type_id_t)
 */
C_TYPE_INLINE
void c_type_id_data_free( void *data ) {
#if SIZEOF_C_TYPE_ID_T > SIZEOF_VOIDP
  free( data );
#else
  (void)data;
#endif /* SIZEOF_C_TYPE_ID_T > SIZEOF_VOIDP */
}

/**
 * Gets the `c_type_id_t` value from a `void*` data value.
 *
 * @param data The data to get the `c_type_id_t` from.
 * @return Returns the `c_type_id_t`.
 *
 * @sa c_type_id_data_new(c_type_id_t)
 */
C_WARN_UNUSED_RESULT C_TYPE_INLINE
c_type_id_t c_type_id_data_get( void *data ) {
#if SIZEOF_C_TYPE_ID_T > SIZEOF_VOIDP
  return *REINTERPRET_CAST( c_type_id_t*, data );
#else
  return REINTERPRET_CAST( c_type_id_t, data );
#endif /* SIZEOF_C_TYPE_ID_T > SIZEOF_VOIDP */
}

/**
 * Creates an opaque data handle for a `c_type_id_t`.
 * @note Callers _must_ eventually call c_type_id_data_free(void*) on the
 * returned value.
 *
 * @param type_id The `c_type_id_t` to use.
 * @return Returns said handle.
 *
 * @sa c_type_id_data_free(void*)
 */
C_WARN_UNUSED_RESULT C_TYPE_INLINE
void* c_type_id_data_new( c_type_id_t type_id ) {
#if SIZEOF_C_TYPE_ID_T > SIZEOF_VOIDP
  c_type_id_t *const data = MALLOC( c_type_id_t, 1 );
  *data = type_id;
  return data;
#else
  return REINTERPRET_CAST( void*, type_id );
#endif /* SIZEOF_C_TYPE_ID_T > SIZEOF_VOIDP */
}

////////// extern functions ///////////////////////////////////////////////////

/**
 * Adds a type to an existing type, e.g., `short` to `int`, ensuring that a
 * particular type is never added more than once, e.g., `short` to `short int`.
 *
 * A special case has to be made for `long` to allow for `long long` yet not
 * allow for `long long long`.
 *
 * @param dest_type A pointer to the <code>\ref c_type_id_t</code> to add to.
 * @param new_type The <code>\ref c_type_id_t</code> to add.
 * @param new_loc The source location of \a new_type.
 * @return Returns `true` only if the type added successfully.
 */
C_WARN_UNUSED_RESULT
bool c_type_add( c_type_id_t *dest_type, c_type_id_t new_type,
                 c_loc_t const *new_loc );

/**
 * Checks that \a type_id is valid.
 *
 * @param type_id The <code>\ref c_type_id_t</code> to check.
 * @return Returns the bitwise-or of the language(s) \a type_id is legal in.
 */
C_WARN_UNUSED_RESULT
c_lang_id_t c_type_check( c_type_id_t type_id );

/**
 * Gets the name of \a type_id.
 *
 * @param type_id The <code>\ref c_type_id_t</code> to get the name of.
 * @return Returns said name.
 * @warning The pointer returned is to a static buffer, so you can't do
 * something like call this twice in the same `printf()` statement.
 */
C_WARN_UNUSED_RESULT
char const* c_type_name( c_type_id_t type_id );

/**
 * Gets the name of \a type_id for part of an error message.  If translating
 * from English to gibberish and the type has an English alias, return the
 * alias, e.g., `non-returning` rather than `noreturn`.
 *
 * @param type_id The <code>\ref c_type_id_t</code> to get the name of.
 * @return Returns said name.
 * @warning The pointer returned is to a static buffer, so you can't do
 * something like call this twice in the same `printf()` statement.
 */
C_WARN_UNUSED_RESULT
char const* c_type_name_error( c_type_id_t type_id );

///////////////////////////////////////////////////////////////////////////////

/** @} */

_GL_INLINE_HEADER_END

#endif /* cdecl_c_type_H */
/* vim:set et sw=2 ts=2: */
