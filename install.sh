#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

source install.options

cp _yed ${bin_dir}/yed
echo "Installed 'yed':       ${bin_dir}"
cp libyed.so ${lib_dir}
echo "Installed 'libyed.so': ${lib_dir}"
mkdir -p ${inc_dir}/yed
rm -rf ${inc_dir}/yed/*
cp src/*.h ${inc_dir}/yed
echo "Installed headers:     ${inc_dir}/yed"
mkdir -p ${plug_dir}
cp plugins/*.so ${plug_dir}
echo "Installed plugins:     ${plug_dir}"
