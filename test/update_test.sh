#! /bin/sh
##
#       cdecl -- C gibberish translator
#       test/update_test.sh
#
#       Copyright (C) 2020-2022  Paul J. Lucas
#
#       This program is free software: you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation, either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

# Uncomment the following line for shell tracing.
#set -x

########## Functions ##########################################################

local_basename() {
  ##
  # Autoconf, 11.15:
  #
  # basename
  #   Not all hosts have a working basename. You can use expr instead.
  ##
  expr "//$1" : '.*/\(.*\)'
}

fail() {
  echo FAIL $TEST_NAME
}

usage() {
  cat >&2 <<END
usage: $ME -s srcdir test ...
END
  exit 1
}

########## Process command-line ###############################################

while getopts s: opt
do
  case $opt in
  s) BUILD_SRC=$OPTARG ;;
  ?) usage ;;
  esac
done
shift `expr $OPTIND - 1`

[ $# -ge 1 ] || usage

########## Initialize #########################################################

##
# The automake framework sets $srcdir. If it's empty, it means this script was
# called by hand, so set it ourselves.
##
[ "$srcdir" ] || srcdir="."

##
# Must put BUILD_SRC first in PATH so we get the correct version of cdecl.
##
PATH=$BUILD_SRC:$PATH

DATA_DIR=$srcdir/data
EXPECTED_DIR=$srcdir/expected
ACTUAL_OUTPUT=/tmp/cdecl_test_output_$$_

trap "x=$?; rm -f /tmp/*_$$_* 2>/dev/null; exit $x" EXIT HUP INT TERM

########## Update tests #######################################################

update_cdecl_test() {
  TEST_PATH=$1
  TEST_NAME=`local_basename "$TEST_PATH"`
  EXPECTED_OUTPUT="$EXPECTED_DIR/`echo $TEST_NAME | sed s/test$/out/`"

  IFS_old=$IFS
  IFS='@'; read COMMAND CONFIG OPTIONS INPUT EXPECTED_EXIT < $TEST_PATH
  [ "$IFS_old" ] && IFS=$IFS_old

  COMMAND=`echo $COMMAND`               # trims whitespace
  CONFIG=`echo $CONFIG`                 # trims whitespace
  [ "$CONFIG" ] && CONFIG="-c $DATA_DIR/$CONFIG"
  # INPUT=`echo $INPUT`                 # don't expand shell metas
  EXPECTED_EXIT=`echo $EXPECTED_EXIT`   # trims whitespace

  echo $TEST_NAME

  #echo "$INPUT" \| $COMMAND $CONFIG "$OPTIONS" \> $ACTUAL_OUTPUT
  echo "$INPUT" | sed 's/^ //' | $COMMAND $CONFIG $OPTIONS > $ACTUAL_OUTPUT 2>&1
  ACTUAL_EXIT=$?

  if [ $ACTUAL_EXIT -eq $EXPECTED_EXIT ]
  then mv $ACTUAL_OUTPUT $EXPECTED_OUTPUT
  else fail
  fi
}

for TEST in $*
do update_cdecl_test $TEST
done

# vim:set et sw=2 ts=2:
