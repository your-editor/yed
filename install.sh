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

if pgrep yed > /dev/null 2>&1; then
    echo "install.sh: [!] It appears that yed is currently running."
    echo "                Installing will crash existing instances."
    read -r -p "${1:-                Install anyway? [y/N]} " response
    case "$response" in
        [yY][eE][sS]|[yY])
            ;;
        *)
            echo "                Install CANCELLLED."
            exit 1
            ;;
    esac
fi

source install.options

mkdir -p ${prefix} || exit 1

cur_run_path=$(readelf -d _yed | sed -e '/RUNPATH/{s~.*\[\(.*\)\]~\1~;n};d')
if [ "$cur_run_path" != "${lib_dir}" ]; then
    echo "install.sh: [!] RUNPATH of currently built yed does not match \$lib_dir from install.options..."

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
                echo "                RUNPATH patched and is not ${lib_dir}."
                ;;
        esac
    else
        echo "                Can't use patchelf to change RUNPATH because patchelf wasn't found."
        echo "                Rebuild yed to use the current \$lib_dir as its RUNPATH or install patchelf."
        exit 1
    fi
fi

cp -f _yed ${bin_dir}/yed || exit 1
echo "Installed 'yed':                 ${bin_dir}"

cp -f libyed.so ${lib_dir} || exit 1
echo "Installed 'libyed.so':           ${lib_dir}"

patch_offset=$(strings -t d ${lib_dir}/libyed.so | grep qrsnhyg_cyht_qve | awk '{ print $1; }')
dd if=<(printf "${plug_dir}") of="${lib_dir}/libyed.so" obs=1 seek=${patch_offset} conv=notrunc status=none || exit 1
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
    if [ "$(basename $plug)" = "init.so" ]; then
        continue
    fi

    dst_dir="${plug_dir}/$(dirname "${plug#plugins/}")"
    mkdir -p ${dst_dir} || exit 1
    cp -f ${plug} ${dst_dir} || exit 1
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
