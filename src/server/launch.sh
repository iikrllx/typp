#!/bin/bash
#
# Launch server, environment
#

gcc -g3 -O0 -Wall server.c -o typp-server -pthread

while true; do
  if test $(pidof typp-server); then
    kill $(pidof typp-server)
  else
    ./typp-server &
    break
  fi
done

usr_path=/usr/local/bin
if test ! $(diff sort-values.sh $usr_path/sort-values.sh &>/dev/null); then
  sudo cp sort-values.sh $usr_path
fi
