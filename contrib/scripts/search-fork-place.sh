#!/bin/bash

usage() {
  cat >&2 <<EOF
Usage: search-fork-place.sh BASEDIR FORKDIR [FILE]

EOF
  exit 1
}

if test $# -lt 2 || ! test -d "$1" || ! test -d "$2"
then usage; 
fi

base="$1"
fork=$(readlink -f "$2")
file="$3"
branch=master

pushd "$base" >/dev/null
git checkout $branch

compare() {
  file=$1
  hash=$(git rev-parse HEAD)
  if test -z "$file"; then
    echo $(diff --exclude .git -r . "$fork"|wc -l) $hash
  else
    echo $(diff "./$file" "$fork/$file"|wc -l) $hash
  fi
}

compare "$file"
while git checkout HEAD^ 2>/dev/null >/dev/null; do 
  compare "$file"
done

popd >/dev/null

