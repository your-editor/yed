#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

source install.options

cp _yed ${bin_dir}/yed
echo "Installed 'yed':       ${bin_dir}"
cp libyed.so ${lib_dir}
echo "Installed 'libyed.so': ${lib_dir}"
mkdir -p ${inc_dir}
rm -rf ${inc_dir}/*
cp src/*.h ${inc_dir}
echo "Installed headers:     ${inc_dir}"
mkdir -p ${plug_dir}
cp plugins/*.so ${plug_dir}
echo "Installed plugins:     ${plug_dir}"
