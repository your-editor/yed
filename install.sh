#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if [ "$(uname)" == "Darwin" ]; then
    function realpath() {
        [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
    }
fi

function dd_fail() {
    echo "install.sh: [!] dd failed"
    exit 1
}


while getopts "c:p:" flag
do
    case "${flag}" in
        c) cfg_name=${OPTARG};;
        p) prefix=$(realpath ${OPTARG});;
        \?) echo "usage: $0 [-c CONFIG] [-p PREFIX]" && exit 1;;
    esac
done

if ! [ -f install.options ]; then
    echo "install.sh: [!] Missing 'install.options'."
    exit 1
fi

source install.options

mkdir -p ${lib_dir} || exit 1
mkdir -p ${plug_dir} || exit 1
mkdir -p ${inc_dir}/yed || exit 1
mkdir -p ${bin_dir} || exit 1
mkdir -p ${share_dir}/yed || exit 1

if [ -z $cfg_name ]; then
    cfg_name="release"
fi

if [ "$(uname)" == "Darwin" ]; then
    DTAGS="-headerpad_max_install_names"
else
    DTAGS="--enable-new-dtags"
fi

export CC=${CC}
if [ $(uname) = "Darwin" ]; then
    if uname -a | grep "arm64" >/dev/null 2>&1; then
        CC+=" -arch arm64"
    fi
fi
export LIB_C_FLAGS="-rdynamic -shared -fPIC -ldl -lm -lpthread"
export DRIVER_C_FLAGS="-rdynamic -ldl -lm -lpthread -Wl,${DTAGS},-rpath,$(realpath ${lib_dir})"

strnstr_test_prg="#include <string.h>\nint main() { strnstr(\"haystack\", \"needle\", 8); return 0; }"
if ! echo -e "${strnstr_test_prg}" | cc -Wall -x c -o /dev/null > /dev/null 2>&1 -; then
    LIB_C_FLAGS+=" -DNEED_STRNSTR"
fi

# Add this framework to the Mac debug build so
# that we can use Instruments.app to profile yed
if [ "$(uname)" == "Darwin" ]; then
    debug+=" -framework CoreFoundation"
fi

cfg=${!cfg_name}

# I hate getting spammed with error messages
ERR_LIM=" -fmax-errors=3 -Wno-unused-command-line-argument"

${CC} --version ${ERR_LIM} 2>&1 > /dev/null

if [ $? -eq 0 ]; then
    cfg+=${ERR_LIM}
fi

export cfg=${cfg}



echo "Compiling libyed.so.."
${CC} src/yed.c -o ${lib_dir}/libyed.so.new ${LIB_C_FLAGS} ${cfg} || exit $?

if [ "${strip}x" = "yesx" ]; then
    echo "    stripped libyed.so"
    strip ${lib_dir}/libyed.so.new
fi

# Patch default_plug_dir in lib
patch_offset=$(strings -t d ${lib_dir}/libyed.so.new | grep qrsnhyg_cyht_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf $(realpath "${plug_dir}")) of="${lib_dir}/libyed.so.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched default plugin path"

# Patch installed_lib_dir in lib
patch_offset=$(strings -t d ${lib_dir}/libyed.so.new | grep vafgnyyrq_yvo_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf $(realpath "${lib_dir}")) of="${lib_dir}/libyed.so.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed lib path"

# Patch installed_include_dir in lib
patch_offset=$(strings -t d ${lib_dir}/libyed.so.new | grep vafgnyyrq_vapyhqr_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf $(realpath "${inc_dir}")) of="${lib_dir}/libyed.so.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed include path"

# Patch installed_share_dir in lib
patch_offset=$(strings -t d ${lib_dir}/libyed.so.new | grep vafgnyyrq_funer_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${lib_dir}/libyed.so.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf $(realpath "${share_dir}")) of="${lib_dir}/libyed.so.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed share path"

if [ $(uname) = "Darwin" ]; then
    install_name_tool -id $(realpath "${lib_dir}/libyed.so") "${lib_dir}/libyed.so.new"
    codesign -s - -f ${lib_dir}/libyed.so.new >/dev/null 2>&1 || exit 1
    echo "    performed codesign fixup"
fi

mv ${lib_dir}/libyed.so.new ${lib_dir}/libyed.so || exit 1
echo "Installed 'libyed.so':             ${lib_dir}"

echo "Creating include directory.."
cp src/*.h ${inc_dir}/yed || exit 1
echo "Installed headers:                 ${inc_dir}/yed"

echo "Compiling the driver.."

${CC} src/yed_driver.c -o ${bin_dir}/yed.new ${DRIVER_C_FLAGS} ${cfg} || exit $?

if [ "${strip}x" = "yesx" ]; then
    echo "    stripped yed"
    strip ${bin_dir}/yed.new
fi

# Patch installed_lib_dir in driver
patch_offset=$(strings -t d ${bin_dir}/yed.new | grep vafgnyyrq_yvo_qve | awk '{ print $1 - 4096; }')
dd if=<(head -c 4096 /dev/zero) of="${bin_dir}/yed.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
dd if=<(printf $(realpath "${lib_dir}")) of="${bin_dir}/yed.new" obs=1 seek=${patch_offset} conv=notrunc >/dev/null 2>&1 || dd_fail
echo "    patched installed lib path"

if [ $(uname) = "Darwin" ]; then
    codesign -s - -f ${bin_dir}/yed.new >/dev/null 2>&1 || exit 1
    echo "    performed codesign fixup"
fi

mv ${bin_dir}/yed.new ${bin_dir}/yed || exit 1
echo "Installed 'yed':                   ${bin_dir}"

echo "Creating default configuration.."
cp -r share/* ${share_dir}/yed
${CC} ${share_dir}/yed/start/init.c -o ${share_dir}/yed/start/init.so $(${bin_dir}/yed --print-cflags) $(${bin_dir}/yed --print-ldflags) || exit 1

if ! [ -d plugins ]; then
    echo "Grabbing plugins.."
    git clone https://github.com/kammerdienerb/yed-plugins plugins
else
    cd plugins
    git pull
    cd ${DIR}
fi

echo "Compiling plugins.."
pids=()
plugs=()

cd ${DIR}/plugins

for b in $(find . -name "build.sh" | sed "s#^\./##"); do
    cd ${DIR}/plugins/$(dirname $b)
    d=$(dirname $b)

    PATH="${bin_dir}:${PATH}" bash build.sh &

    pids+=($!)
    plugs+=($(dirname $b))
done

for (( i=0; i<${#pids[*]}; ++i)); do
    wait ${pids[$i]} || exit 1
    mkdir -p $(dirname ${plug_dir}/${plugs[$i]}) || exit 1
    mv ${DIR}/plugins/${plugs[$i]}/$(basename ${plugs[$i]}).so ${plug_dir}/${plugs[$i]}.so || exit 1
    if [ $(uname) = "Darwin" ] && [ -d ${DIR}/plugins/${plugs[$i]}/$(basename ${plugs[$i]}).so.dSYM ]; then
        if [ -d ${plug_dir}/${plugs[$i]}.so.dSYM ]; then
            rm -rf ${plug_dir}/${plugs[$i]}.so.dSYM
        fi
        mv ${DIR}/plugins/${plugs[$i]}/$(basename ${plugs[$i]}).so.dSYM ${plug_dir}/${plugs[$i]}.so.dSYM
    fi
done
echo "Installed plugins:                 ${plug_dir}"
