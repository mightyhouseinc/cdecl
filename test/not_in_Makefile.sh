#! /bin/sh
##
#       cdecl -- C gibberish translator
#       test/not_in_Makefile.sh
#
#       Copyright (C) 2020-2023  Paul J. Lucas
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

# Prints *.test files that exist but are not in Makefile.am.

ls tests/*.test | while read TEST
do fgrep $TEST Makefile.am >/dev/null 2>&1 || echo $TEST
done

# vim:set et sw=2 ts=2:
