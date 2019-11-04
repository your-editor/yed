#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

# cp _yed /usr/local/bin/yed
# cp libyed.so /usr/local/lib

mkdir -p ~/.yed
cp plugins/*.so ~/.yed
