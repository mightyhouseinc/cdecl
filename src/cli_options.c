/*
**      cdecl -- C gibberish translator
**      src/cli_options.c
**
**      Copyright (C) 2017-2022  Paul J. Lucas, et al.
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
 * Defines global variables and functions for command-line options.
 */

// local
#include "pjl_config.h"                 /* must go first */
#include "cli_options.h"
#include "c_lang.h"
#include "c_type.h"
#include "cdecl.h"
#include "color.h"
#include "help.h"
#include "options.h"
#include "print.h"
#include "strbuf.h"
#include "util.h"

/// @cond DOXYGEN_IGNORE

// standard
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>                     /* for isatty(3) */

// in ascending option character ASCII order
#define SOPT_DIGRAPHS       2
#define SOPT_TRIGRAPHS      3
#define SOPT_ALT_TOKENS     a
#ifdef YYDEBUG
#define SOPT_BISON_DEBUG    B
#endif /* YYDEBUG */
#define SOPT_CONFIG         c
#define SOPT_NO_CONFIG      C
#ifdef ENABLE_CDECL_DEBUG
#define SOPT_CDECL_DEBUG    d
#endif /* ENABLE_CDECL_DEBUG */
#define SOPT_EXPLAIN        e
#define SOPT_EAST_CONST     E
#define SOPT_FILE           f
#ifdef ENABLE_FLEX_DEBUG
#define SOPT_FLEX_DEBUG     F
#endif /* ENABLE_FLEX_DEBUG */
#define SOPT_HELP           h
#define SOPT_INTERACTIVE    i
#define SOPT_EXPLICIT_INT   I
#define SOPT_COLOR          k
#define SOPT_OUTPUT         o
#define SOPT_NO_PROMPT      p
#define SOPT_NO_SEMICOLON   s
#define SOPT_EXPLICIT_ECSU  S
#define SOPT_NO_TYPEDEFS    t
#define SOPT_NO_USING       u
#define SOPT_VERSION        v
#define SOPT_LANGUAGE       x

#define COPT(X)                   CHARIFY(SOPT_##X)
#define SOPT_HELPER(X)            STRINGIFY(X)
#define SOPT(X)                   SOPT_HELPER(SOPT_##X)

/// @endcond

///////////////////////////////////////////////////////////////////////////////

/**
 * Long command-line options.
 *
 * @sa CLI_OPTIONS_SHORT
 */
static struct option const CLI_OPTIONS_LONG[] = {
  //
  // If this array is modified, also modify CLI_OPTIONS_SHORT, the call(s) to
  // check_mutually_exclusive() in parse_options(), the message in usage(), and
  // the corresponding "set" option in SET_OPTIONS in set_options.c.
  //
  { "alt-tokens",     no_argument,        NULL, COPT(ALT_TOKENS)    },
#ifdef YYDEBUG
  { "bison-debug",    no_argument,        NULL, COPT(BISON_DEBUG)   },
#endif /* YYDEBUG */  
  { "color",          required_argument,  NULL, COPT(COLOR)         },
  { "config",         required_argument,  NULL, COPT(CONFIG)        },
#ifdef ENABLE_CDECL_DEBUG
  { "debug",          no_argument,        NULL, COPT(CDECL_DEBUG)   },
#endif /* ENABLE_CDECL_DEBUG */
  { "digraphs",       no_argument,        NULL, COPT(DIGRAPHS)      },
  { "east-const",     no_argument,        NULL, COPT(EAST_CONST)    },
  { "explain",        no_argument,        NULL, COPT(EXPLAIN)       },
  { "explicit-ecsu",  required_argument,  NULL, COPT(EXPLICIT_ECSU) },
  { "explicit-int",   required_argument,  NULL, COPT(EXPLICIT_INT)  },
  { "file",           required_argument,  NULL, COPT(FILE)          },
#ifdef ENABLE_FLEX_DEBUG
  { "flex-debug",     no_argument,        NULL, COPT(FLEX_DEBUG)    },
#endif /* ENABLE_FLEX_DEBUG */
  { "help",           no_argument,        NULL, COPT(HELP)          },
  { "interactive",    no_argument,        NULL, COPT(INTERACTIVE)   },
  { "language",       required_argument,  NULL, COPT(LANGUAGE)      },
  { "no-config",      no_argument,        NULL, COPT(NO_CONFIG)     },
  { "no-prompt",      no_argument,        NULL, COPT(NO_PROMPT)     },
  { "no-semicolon",   no_argument,        NULL, COPT(NO_SEMICOLON)  },
  { "no-typedefs",    no_argument,        NULL, COPT(NO_TYPEDEFS)   },
  { "no-using",       no_argument,        NULL, COPT(NO_USING)      },
  { "output",         required_argument,  NULL, COPT(OUTPUT)        },
  { "trigraphs",      no_argument,        NULL, COPT(TRIGRAPHS)     },
  { "version",        no_argument,        NULL, COPT(VERSION)       },
  { NULL,             0,                  NULL, 0                   }
};

/// @cond DOXYGEN_IGNORE
#define SOPT_NO_ARGUMENT          /* nothing */
#define SOPT_REQUIRED_ARGUMENT    ":"
/// @endcond

/**
 * Short command-line options.
 *
 * @note It _must_ start with `:` to make `getopt_long()` return `:` when a
 * required argument for a known option is missing.
 *
 * @sa CLI_OPTIONS_LONG
 */
static char const   CLI_OPTIONS_SHORT[] = ":"
  SOPT(ALT_TOKENS)    SOPT_NO_ARGUMENT
#ifdef YYDEBUG
  SOPT(BISON_DEBUG)   SOPT_NO_ARGUMENT
#endif /* YYDEBUG */
#ifdef ENABLE_CDECL_DEBUG
  SOPT(CDECL_DEBUG)   SOPT_NO_ARGUMENT
#endif /* ENABLE_CDECL_DEBUG */
  SOPT(COLOR)         SOPT_REQUIRED_ARGUMENT
  SOPT(CONFIG)        SOPT_REQUIRED_ARGUMENT
  SOPT(DIGRAPHS)      SOPT_NO_ARGUMENT
  SOPT(EAST_CONST)    SOPT_NO_ARGUMENT
  SOPT(EXPLAIN)       SOPT_NO_ARGUMENT
  SOPT(EXPLICIT_ECSU) SOPT_REQUIRED_ARGUMENT
  SOPT(EXPLICIT_INT)  SOPT_REQUIRED_ARGUMENT
  SOPT(FILE)          SOPT_REQUIRED_ARGUMENT
#ifdef ENABLE_FLEX_DEBUG
  SOPT(FLEX_DEBUG)    SOPT_NO_ARGUMENT
#endif /* ENABLE_FLEX_DEBUG */
  SOPT(HELP)          SOPT_NO_ARGUMENT
  SOPT(INTERACTIVE)   SOPT_NO_ARGUMENT
  SOPT(LANGUAGE)      SOPT_REQUIRED_ARGUMENT
  SOPT(NO_CONFIG)     SOPT_NO_ARGUMENT
  SOPT(NO_PROMPT)     SOPT_NO_ARGUMENT
  SOPT(NO_SEMICOLON)  SOPT_NO_ARGUMENT
  SOPT(NO_TYPEDEFS)   SOPT_NO_ARGUMENT
  SOPT(NO_USING)      SOPT_NO_ARGUMENT
  SOPT(OUTPUT)        SOPT_REQUIRED_ARGUMENT
  SOPT(TRIGRAPHS)     SOPT_NO_ARGUMENT
  SOPT(VERSION)       SOPT_NO_ARGUMENT
;

// local variables
static bool         opts_given[ 128 ];  ///< Table of options that were given.

// local functions
PJL_WARN_UNUSED_RESULT
static char const*  opt_format( char, strbuf_t* );

PJL_WARN_UNUSED_RESULT
static char const*  opt_get_long( char );

noreturn
static void         usage( int );

////////// local functions ////////////////////////////////////////////////////

/**
 * Checks that no options were given that are among the two given mutually
 * exclusive sets of short options.
 * Prints an error message and exits if any such options are found.
 *
 * @param opts1 The first set of short options.
 * @param opts2 The second set of short options.
 */
static void check_mutually_exclusive( char const *opts1, char const *opts2 ) {
  assert( opts1 != NULL );
  assert( opts2 != NULL );

  unsigned gave_count = 0;
  char const *opt = opts1;
  char gave_opt1 = '\0';

  for ( unsigned i = 0; i < 2; ++i ) {
    for ( ; *opt != '\0'; ++opt ) {
      if ( opts_given[ (unsigned char)*opt ] ) {
        if ( ++gave_count > 1 ) {
          char const gave_opt2 = *opt;
          strbuf_t opt1_sbuf, opt2_sbuf;
          FATAL_ERR( EX_USAGE,
            "%s and %s are mutually exclusive\n",
            opt_format( gave_opt1, &opt1_sbuf ),
            opt_format( gave_opt2, &opt2_sbuf )
          );
        }
        gave_opt1 = *opt;
        break;
      }
    } // for
    if ( gave_count == 0 )
      break;
    opt = opts2;
  } // for
}

/**
 * Checks whether we're c++decl.
 *
 * @param prog_name The name of the program.
 * @returns Returns `true` only if we are.
 */
PJL_WARN_UNUSED_RESULT
static bool is_cppdecl( char const *prog_name ) {
  static char const *const NAMES[] = {
    CPPDECL,
    "cppdecl",
    "cxxdecl",
    NULL
  };

  for ( char const *const *pname = NAMES; *pname != NULL; ++pname ) {
    if ( strcasecmp( *pname, prog_name ) == 0 )
      return true;
  } // for
  return false;
}

/**
 * Formats an option as `[--%%s/]-%%c` where `%%s` is the long option (if any)
 * and `%%c` is the short option.
 *
 * @param short_opt The short option (along with its corresponding long option,
 * if any) to format.
 * @param sbuf A pointer to the strbuf to use.
 * @return Returns \a sbuf->str.
 */
PJL_WARN_UNUSED_RESULT
static char const* opt_format( char short_opt, strbuf_t *sbuf ) {
  assert( sbuf != NULL );
  strbuf_init( sbuf );
  char const *const long_opt = opt_get_long( short_opt );
  strbuf_printf(
    sbuf, "%s%s%s-%c",
    long_opt[0] != '\0' ? "--" : "",
    long_opt,
    long_opt[0] != '\0' ? "/" : "",
    short_opt
  );
  return sbuf->str;
}

/**
 * Gets the corresponding name of the long option for \a short_opt.
 *
 * @param short_opt The short option to get the corresponding long option for.
 * @return Returns said option or the empty string if none.
 */
PJL_WARN_UNUSED_RESULT
static char const* opt_get_long( char short_opt ) {
  FOREACH_CLI_OPTION( opt ) {
    if ( opt->val == short_opt )
      return opt->name;
  } // for
  return "";
}

/**
 * Parses a color "when" value.
 *
 * @param when The null-terminated "when" string to parse.
 * @return Returns the associated \ref color_when or prints an error message
 * and exits if \a when is invalid.
 */
PJL_WARN_UNUSED_RESULT
static color_when_t parse_color_when( char const *when ) {
  struct colorize_map {
    char const   *map_when;
    color_when_t  map_colorization;
  };
  typedef struct colorize_map colorize_map_t;

  static colorize_map_t const COLORIZE_MAP[] = {
    { "always",    COLOR_ALWAYS   },
    { "auto",      COLOR_ISATTY   },    // grep compatibility
    { "isatty",    COLOR_ISATTY   },    // explicit synonym for auto
    { "never",     COLOR_NEVER    },
    { "not_file",  COLOR_NOT_FILE },    // !ISREG( stdout )
    { "not_isreg", COLOR_NOT_FILE },    // synonym for not_isfile
    { "tty",       COLOR_ISATTY   },    // synonym for isatty
    { NULL,        COLOR_NEVER    }
  };

  assert( when != NULL );

  for ( colorize_map_t const *m = COLORIZE_MAP; m->map_when != NULL; ++m ) {
    if ( strcasecmp( when, m->map_when ) == 0 )
      return m->map_colorization;
  } // for

  // name not found: construct valid name list for an error message
  strbuf_t when_sbuf;
  strbuf_init( &when_sbuf );
  bool comma = false;
  for ( colorize_map_t const *m = COLORIZE_MAP; m->map_when != NULL; ++m )
    strbuf_sepsn_puts( &when_sbuf, ", ", 2, &comma, m->map_when );

  strbuf_t opt_sbuf;
  FATAL_ERR( EX_USAGE,
    "\"%s\": invalid value for %s; must be one of: %s\n",
    when, opt_format( COPT(COLOR), &opt_sbuf ), when_sbuf.str
  );
}

/**
 * Parses a language name.
 *
 * @param lang_name The null-terminated name to parse.
 * @return Returns the \ref c_lang_id_t corresponding to \a lang_name.
 */
PJL_WARN_UNUSED_RESULT
static c_lang_id_t parse_lang( char const *lang_name ) {
  assert( lang_name != NULL );

  c_lang_id_t const lang_id = c_lang_find( lang_name );
  if ( lang_id != LANG_NONE )
    return lang_id;

  strbuf_t langs_sbuf;
  strbuf_init( &langs_sbuf );
  bool comma = false;
  FOREACH_LANG( lang ) {
    if ( !lang->is_alias )
      strbuf_sepsn_puts( &langs_sbuf, ", ", 2, &comma, lang->name );
  } // for

  strbuf_t opt_sbuf;
  FATAL_ERR( EX_USAGE,
    "\"%s\": invalid value for %s; must be one of: %s\n",
    lang_name, opt_format( COPT(LANGUAGE), &opt_sbuf ), langs_sbuf.str
  );
}

/**
 * Parses command-line options.
 *
 * @param argc The argument count from main().
 * @param argv The argument values from main().
 */
static void parse_options( int argc, char const *argv[] ) {
  opterr = 0;                           // suppress default error message
  optind = 1;

  color_when_t  color_when = COLOR_WHEN_DEFAULT;
  char const   *fin_path = "-";
  char const   *fout_path = "-";
  bool          print_usage = false;
  bool          print_version = false;

  for (;;) {
    int const opt = getopt_long(
      argc, CONST_CAST( char**, argv ), CLI_OPTIONS_SHORT, CLI_OPTIONS_LONG,
      /*longindex=*/NULL
    );
    if ( opt == -1 )
      break;
    switch ( opt ) {
      case COPT(ALT_TOKENS):
        opt_alt_tokens = true;
        break;
#ifdef YYDEBUG
      case COPT(BISON_DEBUG):
        opt_bison_debug = true;
        break;
#endif /* YYDEBUG */
#ifdef ENABLE_CDECL_DEBUG
      case COPT(CDECL_DEBUG):
        opt_cdecl_debug = true;
        break;
#endif /* ENABLE_CDECL_DEBUG */
      case COPT(COLOR):
        color_when = parse_color_when( optarg );
        break;
      case COPT(CONFIG):
        opt_conf_file = optarg;
        break;
      case COPT(DIGRAPHS):
        opt_graph = C_GRAPH_DI;
        break;
      case COPT(EAST_CONST):
        opt_east_const = true;
        break;
      case COPT(EXPLAIN):
        opt_explain = true;
        break;
      case COPT(EXPLICIT_ECSU):
        if ( !parse_explicit_ecsu( optarg ) ) {
          strbuf_t opt_sbuf;
          FATAL_ERR( EX_USAGE,
            "\"%s\": invalid value for %s;"
            " must be only a combination of: e, c, s, or u\n",
            optarg, opt_format( COPT(EXPLICIT_ECSU), &opt_sbuf )
          );
        }
        break;
      case COPT(EXPLICIT_INT):
        if ( !parse_explicit_int( optarg ) ) {
          strbuf_t opt_sbuf;
          FATAL_ERR( EX_USAGE,
            "\"%s\": invalid value for %s;"
            " must be one of: i, u, or {[u]{isl[l]}[,]}+\n",
            optarg, opt_format( COPT(EXPLICIT_INT), &opt_sbuf )
          );
        }
        break;
      case COPT(FILE):
        fin_path  = optarg;
        break;
#ifdef ENABLE_FLEX_DEBUG
      case COPT(FLEX_DEBUG):
        opt_flex_debug = true;
        break;
#endif /* ENABLE_FLEX_DEBUG */
      case COPT(HELP):
        print_usage = true;
        break;
      case COPT(INTERACTIVE):
        opt_interactive = true;
        break;
      case COPT(LANGUAGE):
        opt_lang = parse_lang( optarg );
        break;
      case COPT(TRIGRAPHS):
        opt_graph = C_GRAPH_TRI;
        break;
      case COPT(NO_CONFIG):
        opt_read_conf = false;
        break;
      case COPT(NO_PROMPT):
        opt_prompt = false;
        break;
      case COPT(NO_SEMICOLON):
        opt_semicolon = false;
        break;
      case COPT(NO_TYPEDEFS):
        opt_typedefs = false;
        break;
      case COPT(NO_USING):
        opt_using = false;
        break;
      case COPT(OUTPUT):
        fout_path = optarg;
        break;
      case COPT(VERSION):
        print_version = true;
        break;

      case ':': {                       // option missing required argument
        strbuf_t sbuf;
        FATAL_ERR( EX_USAGE,
          "\"%s\" requires an argument\n",
          opt_format( STATIC_CAST( char, optopt ), &sbuf )
        );
      }

      case '?':                         // invalid option
        if ( --optind > 0 ) {           // defensive check
          char const *invalid_opt = argv[ optind ];
          //
          // We can offer "did you mean ...?" suggestions only if the invalid
          // option is a long option.
          //
          if ( invalid_opt != NULL && strncmp( invalid_opt, "--", 2 ) == 0 ) {
            invalid_opt += 2;           // skip over "--"
            EPRINTF( "%s: \"%s\": invalid option", me, invalid_opt );
            if ( !print_suggestions( DYM_CLI_OPTIONS, invalid_opt ) )
              goto use_help;
            EPUTC( '\n' );
            exit( EX_USAGE );
          }
        }
        EPRINTF( "%s: '%c': invalid option", me, STATIC_CAST( char, optopt ) );

use_help:
        print_use_help();
        exit( EX_USAGE );

      default:
        // LCOV_EXCL_START
        if ( isprint( opt ) )
          INTERNAL_ERR(
            "'%c': unaccounted-for getopt_long() return value\n", opt
          );
        INTERNAL_ERR(
          "%d: unaccounted-for getopt_long() return value\n", opt
        );
        // LCOV_EXCL_STOP
    } // switch
    opts_given[ opt ] = true;
  } // for

  check_mutually_exclusive( SOPT(DIGRAPHS), SOPT(TRIGRAPHS) );

  check_mutually_exclusive( SOPT(HELP),
    SOPT(ALT_TOKENS)
#ifdef YYDEBUG
    SOPT(BISON_DEBUG)
#endif /* YYDEBUG */
#ifdef ENABLE_CDECL_DEBUG
    SOPT(CDECL_DEBUG)
#endif /* ENABLE_CDECL_DEBUG */
    SOPT(COLOR)
    SOPT(CONFIG)
    SOPT(DIGRAPHS)
    SOPT(EAST_CONST)
    SOPT(EXPLAIN)
    SOPT(EXPLICIT_ECSU)
    SOPT(EXPLICIT_INT)
    SOPT(FILE)
#ifdef ENABLE_FLEX_DEBUG
    SOPT(FLEX_DEBUG)
#endif /* ENABLE_FLEX_DEBUG */
    SOPT(INTERACTIVE)
    SOPT(LANGUAGE)
    SOPT(NO_CONFIG)
    SOPT(NO_PROMPT)
    SOPT(NO_SEMICOLON)
    SOPT(NO_TYPEDEFS)
    SOPT(NO_USING)
    SOPT(OUTPUT)
    SOPT(TRIGRAPHS)
    SOPT(VERSION)
  );

  check_mutually_exclusive( SOPT(VERSION),
    SOPT(ALT_TOKENS)
#ifdef YYDEBUG
    SOPT(BISON_DEBUG)
#endif /* YYDEBUG */
#ifdef ENABLE_CDECL_DEBUG
    SOPT(CDECL_DEBUG)
#endif /* ENABLE_CDECL_DEBUG */
    SOPT(COLOR)
    SOPT(CONFIG)
    SOPT(DIGRAPHS)
    SOPT(EAST_CONST)
    SOPT(EXPLAIN)
    SOPT(EXPLICIT_ECSU)
    SOPT(EXPLICIT_INT)
    SOPT(FILE)
#ifdef ENABLE_FLEX_DEBUG
    SOPT(FLEX_DEBUG)
#endif /* ENABLE_FLEX_DEBUG */
    SOPT(HELP)
    SOPT(INTERACTIVE)
    SOPT(LANGUAGE)
    SOPT(NO_CONFIG)
    SOPT(NO_PROMPT)
    SOPT(NO_SEMICOLON)
    SOPT(NO_TYPEDEFS)
    SOPT(NO_USING)
    SOPT(OUTPUT)
    SOPT(TRIGRAPHS)
  );

  if ( print_usage )
    usage( argc > 2 ? EX_USAGE : EX_OK );

  if ( print_version ) {
    if ( argc > 2 )                     // cdecl -v foo
      usage( EX_USAGE );
    puts( PACKAGE_STRING );
    exit( EX_OK );
  }

  if ( strcmp( fin_path, "-" ) != 0 &&
       (cdecl_fin = fopen( fin_path, "r" )) == NULL ) {
    FATAL_ERR( EX_NOINPUT, "\"%s\": %s\n", fin_path, STRERROR() );
  }
  if ( strcmp( fout_path, "-" ) != 0 &&
       (cdecl_fout = fopen( fout_path, "w" )) == NULL ) {
    FATAL_ERR( EX_CANTCREAT, "\"%s\": %s\n", fout_path, STRERROR() );
  }

  if ( cdecl_fin == NULL )
    cdecl_fin = stdin;
  if ( cdecl_fout == NULL )
    cdecl_fout = stdout;

  colorize = should_colorize( color_when );
  if ( colorize &&
       !(colors_parse( getenv( "CDECL_COLORS" ) ) ||
         colors_parse( getenv( "GCC_COLORS"   ) )) ) {
    PJL_IGNORE_RV( colors_parse( COLORS_DEFAULT ) );
  }
}

/**
 * Prints the usage message and exits.
 *
 * @param status The status to exit with.  If it is `EX_OK`, prints to standard
 * output; otherwise prints to standard error.
 */
noreturn
static void usage( int status ) {
  fprintf( status == EX_OK ? stdout : stderr,
"usage: %s [options] [command...]\n"
"options:\n"
"  --alt-tokens         (-%c) Print alternative tokens.\n"
#ifdef YYDEBUG
"  --bison-debug        (-%c) Print Bison debug output.\n"
#endif /* YYDEBUG */
"  --color=WHEN         (-%c) Colorize output WHEN [default: not_file].\n"
"  --config=FILE        (-%c) Configuration file path [default: ~/" CONF_FILE_NAME_DEFAULT "].\n"
#ifdef ENABLE_CDECL_DEBUG
"  --debug              (-%c) Print " CDECL " debug output.\n"
#endif /* ENABLE_CDECL_DEBUG */
"  --digraphs           (-%c) Print digraphs.\n"
"  --east-const         (-%c) Print in \"east const\" form.\n"
"  --explain            (-%c) Assume \"explain\" when no other command is given.\n"
"  --explicit-ecsu=WHEN (-%c) Print \"class\", \"struct\", \"union\" explicitly WHEN.\n"
"  --explicit-int=WHEN  (-%c) Print \"int\" explicitly WHEN.\n"
"  --file=FILE          (-%c) Read from FILE [default: stdin].\n"
#ifdef ENABLE_FLEX_DEBUG
"  --flex-debug         (-%c) Print Flex debug output.\n"
#endif /* ENABLE_FLEX_DEBUG */
"  --help               (-%c) Print this help and exit.\n"
"  --interactive        (-%c) Force interactive mode.\n"
"  --language=LANG      (-%c) Use LANG.\n"
"  --no-config          (-%c) Suppress reading configuration file.\n"
"  --no-prompt          (-%c) Suppress prompt.\n"
"  --no-semicolon       (-%c) Suppress printing final semicolon for declarations.\n"
"  --no-typedefs        (-%c) Suppress predefining standard types.\n"
"  --no-using           (-%c) Declare types with typedef, not using, in C++.\n"
"  --output=FILE        (-%c) Write to FILE [default: stdout].\n"
"  --trigraphs          (-%c) Print trigraphs.\n"
"  --version            (-%c) Print version and exit.\n"
"\n"
"Report bugs to: " PACKAGE_BUGREPORT "\n"
PACKAGE_NAME " home page: " PACKAGE_URL "\n",
    me,
    COPT(ALT_TOKENS),
#ifdef YYDEBUG
    COPT(BISON_DEBUG),
#endif /* YYDEBUG */
    COPT(COLOR),
    COPT(CONFIG),
#ifdef ENABLE_CDECL_DEBUG
    COPT(CDECL_DEBUG),
#endif /* ENABLE_CDECL_DEBUG */
    COPT(DIGRAPHS),
    COPT(EAST_CONST),
    COPT(EXPLAIN),
    COPT(EXPLICIT_ECSU),
    COPT(EXPLICIT_INT),
    COPT(FILE),
#ifdef ENABLE_FLEX_DEBUG
    COPT(FLEX_DEBUG),
#endif /* ENABLE_FLEX_DEBUG */
    COPT(HELP),
    COPT(INTERACTIVE),
    COPT(LANGUAGE),
    COPT(NO_CONFIG),
    COPT(NO_PROMPT),
    COPT(NO_SEMICOLON),
    COPT(NO_TYPEDEFS),
    COPT(NO_USING),
    COPT(OUTPUT),
    COPT(TRIGRAPHS),
    COPT(VERSION)
  );
  exit( status );
}

////////// extern functions ///////////////////////////////////////////////////

struct option const* cli_option_next( struct option const *opt ) {
  return opt == NULL ? CLI_OPTIONS_LONG : (++opt)->name == NULL ? NULL : opt;
}

void cli_options_init( int *pargc, char const **pargv[] ) {
  assert( pargc != NULL );
  assert( pargv != NULL );

  me = base_name( (*pargv)[0] );
  opt_lang = is_cppdecl( me ) ? LANG_CPP_NEW : LANG_C_NEW;
#ifdef ENABLE_FLEX_DEBUG
  //
  // When -d is specified, Flex enables debugging by default -- undo that.
  //
  opt_flex_debug = false;
#endif /* ENABLE_FLEX_DEBUG */
  parse_options( *pargc, *pargv );
  if ( !opt_interactive )
    opt_interactive = isatty( fileno( cdecl_fin ) );
  c_lang_set( opt_lang );
  *pargc -= optind;
  *pargv += optind;
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */