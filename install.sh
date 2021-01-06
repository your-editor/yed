#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if ! [ -f install.options ]; then
    echo "install.sh: [!] Missing 'install.options'."
    exit 1
fi

if ! [ -f _yed ] || ! [ -f lib/libyed.so ]; then
    echo "install.sh: [!] yed has not been built. Run build.sh first."
    exit 1
fi

source install.options

mkdir -p ${prefix} || exit 1

mkdir -p ${bin_dir} || exit 1
cp _yed ${bin_dir}/yed.new || exit 1
mv ${bin_dir}/yed.new ${bin_dir}/yed || exit 1
echo "Installed 'yed':                 ${bin_dir}"
# Patch installed_lib_dir in driver
patch_offset=$(strings -t d ${bin_dir}/yed | grep vafgnyyrq_yvo_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${bin_dir}/yed" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${lib_dir}") of="${bin_dir}/yed" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed lib path"

mkdir -p ${lib_dir} || exit 1
cp lib/libyed.so ${lib_dir}/libyed.so.new || exit 1
mv ${lib_dir}/libyed.so.new ${lib_dir}/libyed.so || exit 1
echo "Installed 'libyed.so':           ${lib_dir}"

function dd_fail() {
    echo "install.sh: [!] dd failed"
    exit 1
}

# Patch default_plug_dir in lib
patch_offset=$(strings -t d ${lib_dir}/libyed.so | grep qrsnhyg_cyht_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${plug_dir}") of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched default plugin path"
# Patch installed_lib_dir in lib
patch_offset=$(strings -t d ${lib_dir}/libyed.so | grep vafgnyyrq_yvo_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${lib_dir}") of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed lib path"

mkdir -p ${inc_dir}/yed || exit 1
rm -rf ${inc_dir}/yed/* || exit 1
cp -rf include/yed/* ${inc_dir}/yed || exit 1
echo "Installed headers:               ${inc_dir}/yed"

mkdir -p ${share_dir}/yed || exit 1
rm -rf ${share_dir}/yed/* || exit 1
cp -rf share/* ${share_dir}/yed || exit 1
echo "Installed share items:           ${share_dir}/yed"

mkdir -p ${plug_dir} || exit 1

for plug in $(find plugins -name "*.so" | grep -v "dSYM"); do
    # Don't install example init config.
    if [ "$(basename ${plug})" = "init.so" ]; then
        continue
    fi

    dst_dir="${plug_dir}/$(dirname "${plug#plugins/}")"
    mkdir -p ${dst_dir} || exit 1
    cp ${plug} ${dst_dir}/$(basename ${plug}).new || exit 1
    mv ${dst_dir}/$(basename ${plug}).new ${dst_dir}/$(basename ${plug}) || exit 1
done
echo "Installed plugins:               ${plug_dir}"


cat > "${bin_dir}/yedconf" <<FOOZLE
#!/usr/bin/env bash
if ! which yed > /dev/null 2>&1; then
    echo "yed must be installed to run yedconf"
    exit 1
fi

mkdir -p /tmp/_yedconf
cp -rf ${share_dir}/yed/start/* /tmp/_yedconf

cd /tmp/_yedconf

yed --init=. init.c

echo "generated files:"
echo "    /tmp/_yedconf/init.c"
echo "    /tmp/_yedconf/init.so"
FOOZLE

chmod +x "${bin_dir}/yedconf" || exit 1

echo "Installed 'yedconf':             ${bin_dir}"
