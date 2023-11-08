#!/bin/bash
#
# Sort and added ordinal numbers
#

if test ! -z $1; then
  sort -k2 -n -r $HOME/typp-server/$1 | awk '{print NR " " $s}' > ${1/.*all}.final
fi
