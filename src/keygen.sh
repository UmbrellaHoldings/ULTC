#!/bin/bash
# The script for safe priv/pub key generation.

prog=umbrella-ltcKeyGen

# these names are defined in $prog
pub=pub.key
priv=priv.key

if test -e $pub || test -e $priv
then
  echo "There are $pub/$priv files in the current dir already"
  exit 1
fi

dir=$(dirname "$0")
$dir/$prog
chmod 400 $priv
chmod 444 $pub