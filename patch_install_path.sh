#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

function apath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

function dd_fail() {
    echo "patch_install_path.sh: [!] dd failed"
    exit 1
}

if [[ $# != 2 ]]; then
    echo "usage: $0 BUILT_EXE PREFIX" && exit 1
fi

built_exe=$(apath $1)
prefix=$(apath $2)

lib_dir=$(${built_exe} --print-lib-dir)
echo lib_dir $lib_dir

new_lib_dir=${prefix}/lib
new_plug_dir=${new_lib_dir}/yed/plugins
new_bin_dir=${prefix}/bin
new_inc_dir=${prefix}/include
new_share_dir=${prefix}/share


echo "Patching the install for $built_exe to be at prefix $prefix ..."


# Patch default_plug_dir
patch_offset=$(strings -t d ${built_exe} | grep qrsnhyg_cyht_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${built_exe}" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${new_plug_dir}") of="${built_exe}" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail

patch_offset=$(strings -t d ${lib_dir}/libyed.so | grep qrsnhyg_cyht_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${new_plug_dir}") of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched default plugin path    -> ${new_plug_dir}"

# Patch installed_lib_dir
patch_offset=$(strings -t d ${built_exe} | grep vafgnyyrq_yvo_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${built_exe}" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${new_lib_dir}") of="${built_exe}" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail

patch_offset=$(strings -t d ${lib_dir}/libyed.so | grep vafgnyyrq_yvo_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${new_lib_dir}") of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed lib path     -> ${new_lib_dir}"

# Patch installed_include_dir
patch_offset=$(strings -t d ${built_exe} | grep vafgnyyrq_vapyhqr_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${built_exe}" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${new_inc_dir}") of="${built_exe}" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail

patch_offset=$(strings -t d ${lib_dir}/libyed.so | grep vafgnyyrq_vapyhqr_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${new_inc_dir}") of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed include path -> ${new_inc_dir}"

# Patch installed_share_dir
patch_offset=$(strings -t d ${built_exe} | grep vafgnyyrq_funer_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${built_exe}" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${new_share_dir}") of="${built_exe}" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail

patch_offset=$(strings -t d ${lib_dir}/libyed.so | grep vafgnyyrq_funer_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${new_share_dir}") of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed share path   -> ${new_share_dir}"

if [ $(uname) = "Darwin" ]; then
    codesign -s - -f ${built_exe} >/dev/null 2>&1 || exit $?
    codesign -s - -f ${lib_dir}/libyed.so >/dev/null 2>&1 || exit $?
    echo "    performed codesign fixup"
fi

echo "Done!"
