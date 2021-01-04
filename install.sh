#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if ! [ -f install.options ]; then
    echo "install.sh: [!] Missing 'install.options'."
    exit 1
fi

if ! [ -f _yed ] || ! [ -f libyed.so ]; then
    echo "install.sh: [!] yed has not been built. Run build.sh first."
    exit 1
fi

source install.options

mkdir -p ${prefix} || exit 1

if [ "$(uname)" == "Darwin" ]; then
    cur_run_path=$(otool -l _yed | awk '/cmd LC_RPATH/ { x=1; } /path/ { if (x) { print $2; exit(0); } }')
else
    cur_run_path=$(readelf -d _yed | sed -e '/RUNPATH/{s~.*\[\(.*\)\]~\1~;n};d')
fi

if [ "$cur_run_path" != "${lib_dir}" ]; then
    echo "install.sh: [!] RUNPATH of currently built yed does not match \$lib_dir from install.options..."


    if [ "$(uname)" == "Darwin" ]; then
        read -r -p "${1:-    Use install_name_tool to change the current RUNPATH to \$lib_dir? [Y/n]} " response
        case "$response" in
            [nN])
                echo "                RUNPATH NOT patched."
                echo "                Rebuild yed to use the current \$lib_dir as its RUNPATH."
                exit 1
                ;;
            *)
                install_name_tool -rpath "${cur_run_path}" "${lib_dir}" _yed || exit 1
                echo "                RUNPATH patched and is now ${lib_dir}."
                ;;
        esac
    else
        if which patchelf > /dev/null 2>&1; then
            echo "                Found patchelf executable."
            read -r -p "${1:-    Use patchelf to change the current RUNPATH to \$lib_dir? [Y/n]} " response
            case "$response" in
                [nN])
                    echo "                RUNPATH NOT patched."
                    echo "                Rebuild yed to use the current \$lib_dir as its RUNPATH."
                    exit 1
                    ;;
                *)
                    patchelf --set-rpath "${lib_dir}" _yed || exit 1
                    echo "                RUNPATH patched and is now ${lib_dir}."
                    ;;
            esac
        else
            echo "                Can't use patchelf to change RUNPATH because patchelf wasn't found."
            echo "                Rebuild yed to use the current \$lib_dir as its RUNPATH or install patchelf."
            exit 1
        fi
    fi
fi

mkdir -p ${bin_dir} || exit 1
cp _yed ${bin_dir}/yed.new || exit 1
mv ${bin_dir}/yed.new ${bin_dir}/yed || exit 1
echo "Installed 'yed':                 ${bin_dir}"

mkdir -p ${lib_dir} || exit 1
cp libyed.so ${lib_dir}/libyed.so.new || exit 1
mv ${lib_dir}/libyed.so.new ${lib_dir}/libyed.so || exit 1
echo "Installed 'libyed.so':           ${lib_dir}"

patch_offset=$(strings -t d ${lib_dir}/libyed.so | grep qrsnhyg_cyht_qve | awk '{ print $1; }')

function dd_fail() {
    echo "install.sh: [!] dd failed"
    exit 1
}
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf "${plug_dir}") of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched default plugin path"

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
