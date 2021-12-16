/*
**      cdecl -- C gibberish translator
**      src/autocomplete.c
**
**      Copyright (C) 2017-2021  Paul J. Lucas, et al.
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
 * Defines functions for implementing command-line autocompletion.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "cdecl.h"
#include "c_keyword.h"
#include "c_lang.h"
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
 * Subset of cdecl keywords (that are not cdecl commands, nor `help` nor `set`
 * command arguments, nor C/C++ keywords) that are auto-completable.
 *
 * @sa CDECL_COMMANDS
 * @sa CDECL_KEYWORDS
 * @sa C_KEYWORDS
 */
static c_lang_lit_t const AC_CDECL_KEYWORDS[] = {
  { LANG_CPP_MIN(20),       L_ADDRESS         },
  { LANG_C_CPP_MIN(11,11),  L_ALIGN           },
  { LANG_ANY,               L_APPLE_BLOCK     },
  { LANG_ANY,               L_ARRAY           },
  { LANG_C_MIN(11),         L_ATOMIC          },
  { LANG_ANY,               L_BITS            },
  { LANG_C_CPP_MIN(11,11),  L_BYTES           },
  { LANG_ANY,               L_CAST            },
  { LANG_C_MIN(99),         L_COMPLEX         },
  { LANG_CPP_ANY,           L_CONSTRUCTOR     },
  { LANG_CPP_ANY,           L_CONVERSION      },
  { LANG_CPP_MIN(11),       L_DEPENDENCY      },
  { LANG_CPP_ANY,           L_DESTRUCTOR      },
  { LANG_CPP_MIN(17),       L_DISCARD         },
  { LANG_CPP_MIN(20),       L_EVALUATION      },
  { LANG_CPP_MIN(11),       L_EXCEPT          },
  { LANG_CPP_MIN(11),       L_EXPRESSION      },
  { LANG_ANY,               L_FUNCTION        },
  { LANG_C_MIN(99),         L_IMAGINARY       },
  { LANG_CPP_MIN(20),       L_INITIALIZATION  },
  { LANG_C_MIN(99),         L_LENGTH          },
  { LANG_CPP_ANY,           L_LINKAGE         },
  { LANG_CPP_MIN(11),       L_LITERAL         },
  { LANG_C_CPP_MIN(11,11),  L_LOCAL           },
  { LANG_CPP_ANY,           L_MEMBER          },
  { LANG_CPP_ANY,           H_NON_MEMBER      },
  { LANG_ANY,               L_POINTER         },
  { LANG_ANY,               L_PREDEFINED      },
  { LANG_CPP_ANY,           L_PURE            },
  { LANG_CPP_ANY,           L_REFERENCE       },
  { LANG_ANY,               L_RETURNING       },
  { LANG_CPP_MIN(11),       L_RVALUE          },
  { LANG_CPP_ANY,           L_SCOPE           },
  { LANG_CPP_MIN(20),       L_UNIQUE          },
  { LANG_C_CPP_MIN(2X,17),  L_UNUSED          },
  { LANG_CPP_ANY,           H_USER_DEFINED    },
  { LANG_C_MIN(99),         L_VARIABLE        },
  { LANG_ANY,               L_VARIADIC        },
  { LANG_ANY,               L_VECTOR          },
  { LANG_ANY,               L_WIDTH           },

  { LANG_NONE,              NULL              }
};

// local functions
static char*  command_generator( char const*, int );
static char*  keyword_generator( char const*, int );

PJL_WARN_UNUSED_RESULT
static bool   is_command( char const* );

////////// local functions ////////////////////////////////////////////////////

/**
 * Creates and initializes an array of all auto-completable keywords composed
 * of C/C++ keywords and cdecl keywords.
 *
 * @return Returns a pointer to said array.
 */
PJL_WARN_UNUSED_RESULT
static c_lang_lit_t const* init_cdecl_keywords( void ) {
  size_t cdecl_keywords_size = ARRAY_SIZE( AC_CDECL_KEYWORDS );

  // pre-flight to calculate array size
  FOREACH_C_KEYWORD( k ) {
    if ( k->ac_lang_ids != LANG_NONE )
      ++cdecl_keywords_size;
  } // for

  c_lang_lit_t *const cdecl_keywords =
    free_later( MALLOC( c_lang_lit_t, cdecl_keywords_size ) );
  c_lang_lit_t *p = cdecl_keywords;

  for ( c_lang_lit_t const *ll = AC_CDECL_KEYWORDS; ll->literal != NULL; ++ll )
    *p++ = *ll;

  FOREACH_C_KEYWORD( k ) {
    if ( k->ac_lang_ids != LANG_NONE ) {
      p->lang_ids = k->ac_lang_ids;
      p->literal  = k->literal;
      ++p;
    }
  } // for

  MEM_ZERO( p );

  return cdecl_keywords;
}

/**
 * Creates and initializes an array of all `set` option strings to be used for
 * autocompletion for the `set` command.
 *
 * @return Returns a pointer to said array.
 */
PJL_WARN_UNUSED_RESULT
static char const* const* init_set_options( void ) {
  size_t set_options_size =
      1                                 // for "options"
    + 1;                                // for terminating NULL pointer

  // pre-flight to calculate array size
  FOREACH_SET_OPTION( opt ) {
    set_options_size += 1 + (size_t)(opt->type == SET_OPT_TOGGLE /* "no" */);
  } // for
  FOREACH_LANG( lang ) {
    if ( !lang->is_alias )
      ++set_options_size;
  } // for

  char **const set_options = free_later( MALLOC( char*, set_options_size ) );
  char **p = set_options;

  *p++ = CONST_CAST( char*, L_OPTIONS );

  FOREACH_SET_OPTION( opt ) {
    switch ( opt->type ) {
      case SET_OPT_AFF_ONLY:
        *p++ = CONST_CAST( char*, opt->name );
        break;

      case SET_OPT_TOGGLE:
        *p++ = CONST_CAST( char*, opt->name );
        PJL_FALLTHROUGH;

      case SET_OPT_NEG_ONLY:
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
 * Checks whether the current line being read is a cast command.  In C, this
 * can only be `cast`; in C++, this can also be `const`, `dynamic`, `static`,
 * or `reinterpret`.
 *
 * @return Returns `true` only if it's a cast command.
 *
 * @sa is_command()
 */
PJL_WARN_UNUSED_RESULT
static bool is_cast_command( void ) {
  if ( is_command( L_CAST ) )
    return true;
  if ( OPT_LANG_IS(C_ANY) )
    return false;
  return  is_command( L_CONST       ) ||
          is_command( L_DYNAMIC     ) ||
          is_command( L_STATIC      ) ||
          is_command( L_REINTERPRET );
}

/**
 * Checks whether the current line being read is a particular cdecl command.
 *
 * @param command The command to check for.
 * @return Returns `true` only if it is.
 *
 * @sa is_cast_command()
 */
PJL_WARN_UNUSED_RESULT
static bool is_command( char const *command ) {
  assert( command != NULL );
  size_t const command_len = strlen( command );
  if ( command_len > (size_t)rl_end )   // more chars than in rl_line_buffer?
    return false;
  return strncmp( rl_line_buffer, command, command_len ) == 0;
}

////////// readline callback functions ////////////////////////////////////////

/**
 * Attempts command completion for readline().
 *
 * @param text The text read (so far) to match against.
 * @param start The starting character position of \a text.
 * @param end The ending character position of \a text.
 * @return Returns an array of C strings of possible matches.
 */
static char** cdecl_rl_completion( char const *text, int start, int end ) {
  assert( text != NULL );
  (void)end;

  rl_attempted_completion_over = 1;     // don't do filename completion

  //
  // If the word is at the start of the line (start == 0), attempt to complete
  // only cdecl commands and not all keywords.  Having two generator functions
  // makes the logic simpler in each.
  //
  return rl_completion_matches(
    text, start == 0 ? command_generator : keyword_generator
  );
}

/**
 * Attempts to match a cdecl command.
 *
 * @param text The text read (so far) to match against.
 * @param state If 0, restart matching from the beginning; if non-zero,
 * continue to next match, if any.
 * @return Returns a copy of the command or NULL if none.
 */
static char* command_generator( char const *text, int state ) {
  assert( text != NULL );

  static size_t match_index;
  static size_t text_len;

  if ( state == 0 ) {                   // new word? reset
    match_index = 0;
    text_len = strlen( text );
  }

  for ( cdecl_command_t const *c;
        (c = CDECL_COMMANDS + match_index)->literal != NULL; ) {
    ++match_index;
    if ( !opt_lang_is_any( c->lang_ids ) )
      continue;
    if ( strncmp( text, c->literal, text_len ) == 0 )
      return check_strdup( c->literal );
  } // for

  return NULL;
}

/**
 * Attempts to match a cdecl keyword (that is not a command).
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
    if ( is_command( "?" ) )
      command = L_HELP;
    else if ( is_cast_command() )
      command = L_CAST;
    else {
      command = NULL;
      FOREACH_CDECL_COMMAND( c ) {
        if ( opt_lang_is_any( c->lang_ids ) && is_command( c->literal ) ) {
          command = c->literal;
          break;
        }
      } // for
    }
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
  if ( command == L_CAST &&
       strncmp( text, L_INTO, text_len ) == 0 &&
       strstr( rl_line_buffer, L_INTO ) == NULL ) {
    command = NULL;                     // unambiguously match "into"
    return check_strdup( L_INTO );
  }

  static char const *const *command_keywords;

  if ( state == 0 ) {
    //
    // Special case: for certain commands, complete using specific keywords for
    // that command.
    //
    if ( command == L_HELP ) {
      static char const *const help_keywords[] = {
        L_COMMANDS,
        L_ENGLISH,
        L_OPTIONS,
        NULL
      };
      command_keywords = help_keywords;
    }
    else if ( command == L_SET_COMMAND ) {
      static char const *const *set_options;
      if ( set_options == NULL )
        set_options = init_set_options();
      command_keywords = set_options;
    }
    else {
      command_keywords = NULL;
    }
  }

  if ( command_keywords != NULL ) {
    //
    // There's a special-case command having specific keywords in effect:
    // attempt to match against only those.
    //
    for ( char const *s; (s = command_keywords[ match_index ]) != NULL; ) {
      ++match_index;
      if ( strncmp( text, s, text_len ) == 0 )
        return check_strdup( s );
    } // for
  }
  else {
    //
    // Otherwise, just attempt to match (almost) any keyword.
    //
    static c_lang_lit_t const *cdecl_keywords;
    if ( cdecl_keywords == NULL )
      cdecl_keywords = init_cdecl_keywords();

    for ( c_lang_lit_t const *ll;
          (ll = cdecl_keywords + match_index)->literal != NULL; ) {
      ++match_index;
      if ( !opt_lang_is_any( ll->lang_ids ) )
        continue;
      if ( strncmp( text, ll->literal, text_len ) == 0 )
        return check_strdup( ll->literal );
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
