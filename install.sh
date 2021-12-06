#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

function apath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}


while getopts "c:p:" flag
do
    case "${flag}" in
        c) cfg_name=${OPTARG};;
        p) prefix=$(apath ${OPTARG});;
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

export CC=${CC}
if [ $(uname) = "Darwin" ]; then
    if uname -a | grep "arm64" >/dev/null 2>&1; then
        CC+=" -arch arm64"
    fi
fi

LIB_C_FLAGS="-rdynamic -shared -fPIC -lm -lpthread"
DRIVER_C_FLAGS="-Isrc -rdynamic -lm -lpthread"

strnstr_test_prg="#include <string.h>\nint main() { strnstr(\"haystack\", \"needle\", 8); return 0; }"
if ! echo -e "${strnstr_test_prg}" | cc -Wall -x c -o /dev/null > /dev/null 2>&1 -; then
    LIB_C_FLAGS+=" -DNEED_STRNSTR"
fi

if [ -z "${execinfo_prefix}" ]; then
    backtrace_test_prg="#include <execinfo.h>\nint main() { void *p[1]; backtrace(p, 1); return 0; }"
    if echo -e "${backtrace_test_prg}" | cc -Wall -x c -o /dev/null > /dev/null 2>&1 -; then
        LIB_C_FLAGS+=" -DHAS_BACKTRACE"
        DRIVER_C_FLAGS+=" -DHAS_BACKTRACE"
    fi
else
    LIB_C_FLAGS+=" -DHAS_BACKTRACE -I${execinfo_prefix}/include -L${execinfo_prefix}/lib -lexecinfo"
    DRIVER_C_FLAGS+=" -DHAS_BACKTRACE -I${execinfo_prefix}/include -L${execinfo_prefix}/lib -lexecinfo"
fi

if ! uname | grep "BSD" >/dev/null 2>&1; then
    LIB_C_FLAGS+=" -ldl"
    DRIVER_C_FLAGS+=" -ldl"
fi

LIB_C_FLAGS+=" -DDEFAULT_PLUG_DIR=\"$(apath ${plug_dir})\""
LIB_C_FLAGS+=" -DINSTALLED_LIB_DIR=\"$(apath ${lib_dir})\""
LIB_C_FLAGS+=" -DINSTALLED_INCLUDE_DIR=\"$(apath ${inc_dir})\""
LIB_C_FLAGS+=" -DINSTALLED_SHARE_DIR=\"$(apath ${share_dir})\""

DRIVER_C_FLAGS+=" -DDEFAULT_PLUG_DIR=\"$(apath ${plug_dir})\""
DRIVER_C_FLAGS+=" -DINSTALLED_LIB_DIR=\"$(apath ${lib_dir})\""
DRIVER_C_FLAGS+=" -DINSTALLED_INCLUDE_DIR=\"$(apath ${inc_dir})\""
DRIVER_C_FLAGS+=" -DINSTALLED_SHARE_DIR=\"$(apath ${share_dir})\""

export LIB_C_FLAGS
export DRIVER_C_FLAGS

cfg=${!cfg_name}

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

if [ $(uname) = "Darwin" ]; then
    install_name_tool -id $(apath "${lib_dir}/libyed.so") "${lib_dir}/libyed.so.new"
    codesign -s - -f ${lib_dir}/libyed.so.new >/dev/null 2>&1 || exit 1
    echo "    performed codesign fixup"
fi

mv ${lib_dir}/libyed.so.new ${lib_dir}/libyed.so || exit 1
if [ $(uname) = "Darwin" ] && [ -d "${lib_dir}/libyed.so.new.dSYM" ]; then
    rm -rf "${lib_dir}/libyed.so.dSYM" || exit 1
    mv "${lib_dir}/libyed.so.new.dSYM" "${lib_dir}/libyed.so.dSYM" || exit 1
    mv "${lib_dir}/libyed.so.dSYM/Contents/Resources/DWARF/libyed.so.new" "${lib_dir}/libyed.so.dSYM/Contents/Resources/DWARF/libyed.so" || exit 1
fi
echo "Installed 'libyed.so':             ${lib_dir}"

echo "Creating include directory.."
cp src/*.h ${inc_dir}/yed || exit 1
echo "Installed headers:                 ${inc_dir}/yed"

echo "Compiling the driver.."

${CC} src/yed_driver.c src/whereami.c -o ${bin_dir}/yed.new ${DRIVER_C_FLAGS} ${cfg} || exit $?

if [ "${strip}x" = "yesx" ]; then
    echo "    stripped yed"
    strip ${bin_dir}/yed.new
fi

if [ $(uname) = "Darwin" ]; then
    codesign -s - -f ${bin_dir}/yed.new >/dev/null 2>&1 || exit 1
    echo "    performed codesign fixup"
fi

mv ${bin_dir}/yed.new ${bin_dir}/yed || exit 1
if [ $(uname) = "Darwin" ] && [ -d "${bin_dir}/yed.new.dSYM" ]; then
    rm -rf "${bin_dir}/yed.dSYM" || exit 1
    mv "${bin_dir}/yed.new.dSYM" "${bin_dir}/yed.dSYM" || exit 1
    mv "${bin_dir}/yed.dSYM/Contents/Resources/DWARF/yed.new" "${bin_dir}/yed.dSYM/Contents/Resources/DWARF/yed" || exit 1
fi
echo "Installed 'yed':                   ${bin_dir}"

echo "Creating default configuration.."
cp -r share/* ${share_dir}/yed
${CC} ${share_dir}/yed/start/init.c -o ${share_dir}/yed/start/init.so $(${bin_dir}/yed --print-cflags) $(${bin_dir}/yed --print-ldflags) || exit 1
echo "Installed share items:             ${share_dir}/yed"

MAJOR_VERSION=$(${bin_dir}/yed --major-version)

if ! [ -d plugins ]; then
    echo "Grabbing plugins.."
    git clone https://github.com/your-editor/yed-plugins plugins
    cd plugins
    git checkout "v${MAJOR_VERSION}"
    cd ${DIR}
else
    if [ -d plugins/.git ]; then
        echo "Updating plugins.."
        cd plugins
        git pull
        git checkout "v${MAJOR_VERSION}"
        cd ${DIR}
    else
        echo "Found plugins."
    fi
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
