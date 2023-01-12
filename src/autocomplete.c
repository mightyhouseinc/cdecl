/*
**      cdecl -- C gibberish translator
**      src/autocomplete.c
**
**      Copyright (C) 2017-2023  Paul J. Lucas, et al.
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
 * Defines functions that implement command-line autocompletion.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "c_keyword.h"
#include "c_lang.h"
#include "cdecl_command.h"
#include "cdecl_keyword.h"
#include "literals.h"
#include "options.h"
#include "set_options.h"
#include "util.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>          /* must go last */

#if !HAVE_DECL_RL_COMPLETION_MATCHES
# define rl_completion_matches    completion_matches
#endif /* !HAVE_DECL_RL_COMPLETION_MATCHES */

/// @endcond

///////////////////////////////////////////////////////////////////////////////

/**
 * Auto-completable keyword.
 *
 * This is either a of C/C++ keyword or a **cdecl** keyword that is auto-
 * completable.
 */
struct ac_keyword {
  char const *literal;                  ///< C string literal of the keyword.
  c_lang_id_t ac_lang_ids;              ///< Language(s) auto-completable in.
  bool        always_autocomplete;      ///< Auto-complete even for gibberish?
};
typedef struct ac_keyword ac_keyword_t;

// local functions
static char*              command_generator( char const*, int );
static char*              keyword_generator( char const*, int );

NODISCARD
static bool               is_command( char const*, char const*, size_t );

////////// local functions ////////////////////////////////////////////////////

/**
 * Auto-completion wrapper around cdecl_command_next() that returns only auto-
 * completable **cdecl** commands.
 *
 * @param command A pointer to the previous command. For the first iteration,
 * NULL should be passed.
 * @return Returns the next auto-completable command or NULL for none.
 */
NODISCARD static
cdecl_command_t const* ac_cdecl_command_next( cdecl_command_t const *command ) {
  do {
    command = cdecl_command_next( command );
  } while ( command != NULL && command->ac_lang_ids == LANG_NONE );
  return command;
}

/**
 * Creates and initializes an array of all auto-completable keywords composed
 * of C/C++ keywords and **cdecl** keywords.
 *
 * @return Returns a pointer to said array.
 */
NODISCARD
static ac_keyword_t const* init_ac_keywords( void ) {
  size_t n = 1;                         // for terminating empty element

  // pre-flight to calculate array size
  FOREACH_C_KEYWORD( k )
    n += k->ac_lang_ids != LANG_NONE;
  FOREACH_CDECL_KEYWORD( k )
    n += k->ac_lang_ids != LANG_NONE;

  ac_keyword_t *const ac_keywords = free_later( MALLOC( ac_keyword_t, n ) );
  ac_keyword_t *p = ac_keywords;

  FOREACH_C_KEYWORD( k ) {
    if ( k->ac_lang_ids != LANG_NONE ) {
      p->literal = k->literal;
      p->ac_lang_ids = k->ac_lang_ids;
      p->always_autocomplete = true;
      ++p;
    }
  } // for

  FOREACH_CDECL_KEYWORD( k ) {
    if ( k->ac_lang_ids != LANG_NONE ) {
      p->literal = k->literal;
      p->ac_lang_ids = k->ac_lang_ids;
      p->always_autocomplete = k->always_find;
      ++p;
    }
  } // for

  MEM_ZERO( p );

  return ac_keywords;
}

/**
 * Creates and initializes an array of all `set` option strings to be used for
 * autocompletion for the `set` command.
 *
 * @return Returns a pointer to said array.
 */
NODISCARD
static char const* const* init_set_options( void ) {
  size_t n =
      1                                 // for "options"
    + 1;                                // for terminating NULL pointer

  // pre-flight to calculate array size
  FOREACH_SET_OPTION( opt )
    n += 1u + (opt->kind == SET_OPTION_TOGGLE /* "no" */);
  FOREACH_LANG( lang )
    n += !lang->is_alias;

  char **const set_options = free_later( MALLOC( char*, n ) );
  char **p = set_options;

  *p++ = CONST_CAST( char*, L_options );

  FOREACH_SET_OPTION( opt ) {
    switch ( opt->kind ) {
      case SET_OPTION_AFF_ONLY:
        *p++ = CONST_CAST( char*, opt->name );
        break;

      case SET_OPTION_TOGGLE:
        *p++ = CONST_CAST( char*, opt->name );
        FALLTHROUGH;

      case SET_OPTION_NEG_ONLY:
        *p = free_later(
          MALLOC( char, 2/*no*/ + strlen( opt->name ) + 1/*\0*/ )
        );
        strcpy( *p + 0, "no" );
        strcpy( *p + 2, opt->name );
        ++p;
        break;
    } // switch
  } // for
  FOREACH_LANG( lang ) {
    if ( !lang->is_alias )
      *p++ = free_later( check_strdup_tolower( lang->name ) );
  } // for

  *p = NULL;

  return (char const*const*)set_options;
}

/**
 * Checks whether \a s is a cast command.  In C, this can only be `cast`; in
 * C++, this can also be `const`, `dynamic`, `static`, or `reinterpret`.
 *
 * @param s The string to check.  Leading whitespace must have been skipped.
 * @param s_len The length of \a s.
 * @return Returns `true` only if it's a cast command.
 *
 * @sa is_command()
 */
NODISCARD
static bool is_cast_command( char const *s, size_t s_len ) {
  if ( is_command( L_cast, s, s_len ) )
    return true;
  if ( OPT_LANG_IS( C_ANY ) )
    return false;
  return  is_command( L_const,       s, s_len ) ||
          is_command( L_dynamic,     s, s_len ) ||
          is_command( L_static,      s, s_len ) ||
          is_command( L_reinterpret, s, s_len );
}

/**
 * Checks whether \a s is a particular **cdecl** command.
 *
 * @param command The command to check for.
 * @param s The string to check.  Leading whitespace must have been skipped.
 * @param s_len The length of \a s.
 * @return Returns `true` only if it is.
 *
 * @sa is_cast_command()
 */
NODISCARD
static bool is_command( char const *command, char const *s, size_t s_len ) {
  assert( command != NULL );
  assert( s != NULL );
  size_t const command_len = strlen( command );
  if ( command_len > s_len || strncmp( s, command, command_len ) != 0 )
    return false;
  if ( command_len == s_len )
    return true;
  //
  // If s_len > command_len, then the first character past the end of the
  // command must not be an identifier character.  For example, if command is
  // "foo", then it must be "foo" exactly (above); or "foo" followed by a
  // whitespace or punctuation character, but not an identifier character:
  //
  //      "foo"   match
  //      "foo "  match
  //      "foo("  match
  //      "foob"  no match
  //
  return !is_ident( s[ command_len ] );
}

/**
 * Gets whether \a command is an English command, that is followed by pseudo-
 * English instead of gibberish.
 *
 * @param command The **cdecl** command to check.
 * @return Returns `true` only if \a command is an English command.
 */
NODISCARD
static bool is_english_command( char const *command ) {
  assert( command != NULL );
  return  command == L_cast     ||
          command == L_declare  ||
          command == L_define   ||
          command == L_help     ||
          command == L_set;
}

/**
 * Gets a specific list of keywords to autocomplete for \a command, if any.
 *
 * @param command The command to get the specific list of keywords for.
 * @return Returns a NULL-terminated array of keywords for \a command or NULL
 * if \a command has no specific keywords.
 */
NODISCARD
static char const *const* specific_command_keywords( char const *command ) {
  if ( command == L_help ) {
    static char const *const help_keywords[] = {
      L_commands,
      L_english,
      L_options,
      NULL
    };
    return help_keywords;
  }

  if ( command == L_set ) {
    static char const *const *set_options;
    if ( set_options == NULL )
      set_options = init_set_options();
    return set_options;
  }

  if ( command == L_show ) {
    static char const *const show_keywords[] = {
      L_all,
      L_english,
      L_predefined,
      L_typedef,
      L_user,
      NULL
    };
    static char const *const show_keywords_with_using[] = {
      L_all,
      L_english,
      L_predefined,
      L_typedef,
      L_user,
      L_using,
      NULL
    };
    return OPT_LANG_IS( using_DECLARATION ) ?
      show_keywords_with_using : show_keywords;
  }

  return NULL;
}

////////// readline callback functions ////////////////////////////////////////

/**
 * Attempts command completion for readline().
 *
 * @param text The text read so far, without leading whitespace, if any, to
 * match against.
 * @param start The starting character position of \a text.
 * @param end The ending character position of \a text.
 * @return Returns an array of C strings of possible matches.
 */
static char** cdecl_rl_completion( char const *text, int start, int end ) {
  assert( text != NULL );
  (void)end;

  rl_attempted_completion_over = 1;     // don't do filename completion

  //
  // Determine whether we should complete a cdecl command (the first word on
  // the line) vs. a non-command keyword: if start is zero or all characters in
  // the readline buffer before start are whitespace, then complete a command.
  // Having two generator functions makes the logic simpler in each.
  //
  size_t const n = STATIC_CAST( size_t, start );
  bool const is_command = strnspn( rl_line_buffer, " ", n ) == n;
  return rl_completion_matches(
    text, is_command ? command_generator : keyword_generator
  );
}

/**
 * Attempts to match a **cdecl** command.
 *
 * @param text The text read (so far) to match against.
 * @param state If 0, restart matching from the beginning; if non-zero,
 * continue to next match, if any.
 * @return Returns a copy of the command or NULL if none.
 */
static char* command_generator( char const *text, int state ) {
  assert( text != NULL );

  static cdecl_command_t const *match_command;
  static size_t text_len;

  if ( state == 0 ) {                   // new word? reset
    match_command = ac_cdecl_command_next( NULL );
    text_len = strlen( text );
  }

  while ( match_command != NULL ) {
    cdecl_command_t const *const c = match_command;
    match_command = ac_cdecl_command_next( match_command );
    if ( !opt_lang_is_any( c->lang_ids ) )
      continue;
    if ( strncmp( c->literal, text, text_len ) == 0 )
      return check_strdup( c->literal );
  } // while

  return NULL;
}

/**
 * Attempts to match a **cdecl** keyword (that is not a command).
 *
 * @param text The text read (so far) to match against.
 * @param state If 0, restart matching from the beginning; if non-zero,
 * continue to next match, if any.
 * @return Returns a copy of the keyword or NULL if none.
 */
static char* keyword_generator( char const *text, int state ) {
  assert( text != NULL );

  static char const  *command;          // current command
  static size_t       match_index;
  static size_t       text_len;

  if ( state == 0 ) {                   // new word? reset
    size_t const rl_size = STATIC_CAST( size_t, rl_end );
    size_t const leading_spaces = strnspn( rl_line_buffer, " ", rl_size );
    size_t const buf_len = rl_size - leading_spaces;
    if ( buf_len == 0 )
      return NULL;
    char const *const buf = rl_line_buffer + leading_spaces;

    command = NULL;
    match_index = 0;
    text_len = strlen( text );

    //
    // Retroactively figure out what the current command is so we can do some
    // command-sensitive autocompletion.  We can't just set the command in
    // command_generator() since it may never be called: the user could type an
    // entire command, then hit <tab> sometime later, e.g.:
    //
    //      cdecl> set <tab>
    //
    if ( is_command( "?", buf, buf_len ) )
      command = L_help;
    else if ( is_cast_command( buf, buf_len ) )
      command = L_cast;
    else {
      FOREACH_CDECL_COMMAND( c ) {
        if ( !opt_lang_is_any( c->lang_ids ) )
          continue;
        if ( is_command( c->literal, buf, buf_len ) ) {
          command = c->literal;
          break;
        }
      } // for
    }
    if ( command == NULL && opt_explain )
      command = L_explain;
  }

  if ( command == NULL ) {
    //
    // We haven't at least matched a command yet, so don't match any other
    // keywords.
    //
    return NULL;
  }

  //
  // Special case: if it's the "cast" command, the text partially matches
  // "into", and the user hasn't typed "into" yet, complete as "into".
  //
  if ( command == L_cast &&
       strncmp( L_into, text, text_len ) == 0 &&
       strstr( rl_line_buffer, L_into ) == NULL ) {
    command = NULL;                     // unambiguously match "into"
    return check_strdup( L_into );
  }

  static char const *const *specific_ac_keywords;

  if ( state == 0 ) {
    //
    // Special case: for certain commands, complete using specific keywords for
    // that command.
    //
    specific_ac_keywords = specific_command_keywords( command );
  }

  if ( specific_ac_keywords != NULL ) {
    //
    // There's a special-case command having specific keywords in effect:
    // attempt to match against only those.
    //
    for ( char const *s; (s = specific_ac_keywords[ match_index ]) != NULL; ) {
      ++match_index;
      if ( strncmp( s, text, text_len ) == 0 )
        return check_strdup( s );
    } // for
  }
  else {
    //
    // Otherwise, just attempt to match (almost) any keyword.
    //
    static ac_keyword_t const *ac_keywords;
    if ( ac_keywords == NULL )
      ac_keywords = init_ac_keywords();

    // The keywords following the command are in gibberish, not English.
    bool const is_gibberish = !is_english_command( command );

    for ( ac_keyword_t const *k;
          (k = ac_keywords + match_index)->literal != NULL; ) {
      ++match_index;
      if ( is_gibberish && !k->always_autocomplete ) {
        //
        // The keywords following the command are in gibberish, but the
        // keyword is a cdecl keyword, so skip it.
        //
        continue;
      }
      if ( !opt_lang_is_any( k->ac_lang_ids ) )
        continue;
      if ( strncmp( k->literal, text, text_len ) == 0 )
        return check_strdup( k->literal );
    } // for
  }

  return NULL;
}

////////// extern functions ///////////////////////////////////////////////////

/**
 * Initializes readline.
 *
 * @param rin The `FILE` to read from.
 * @param rout The `FILE` to write to.
 */
void readline_init( FILE *rin, FILE *rout ) {
  // allow conditional ~/.inputrc parsing
  rl_readline_name = CONST_CAST( char*, CDECL );

  rl_attempted_completion_function = cdecl_rl_completion;
  rl_instream = rin;
  rl_outstream = rout;
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
