#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if ! [ -f install.options ]; then
    echo "install.sh: [!] Missing 'install.options'."
    exit 1
fi

source install.options

cp _yed ${bin_dir}/yed
echo "Installed 'yed':       ${bin_dir}"
cp libyed.so ${lib_dir}
echo "Installed 'libyed.so': ${lib_dir}"
mkdir -p ${inc_dir}/yed
rm -rf ${inc_dir}/yed/*
cp include/yed/* ${inc_dir}/yed
echo "Installed headers:     ${inc_dir}/yed"
mkdir -p ${plug_dir}

for plug in $(find plugins -name "*.so" | grep -v "dSYM"); do
    the_plug=""
    dst_dir="${plug_dir}/$(dirname "${plug#plugins/}")"
    mkdir -p ${dst_dir}
    cp ${plug} ${dst_dir}
done
chown -R $(logname) ${plug_dir}
echo "Installed plugins:     ${plug_dir}"
