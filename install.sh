#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

function apath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}


while getopts "c:p:b:" flag
do
    case "${flag}" in
        c) cfg_name=${OPTARG};;
        p) prefix=$(apath ${OPTARG});;
        b) TEMP=$(apath ${OPTARG});;
        \?) echo -e "usage: $0 [-c CONFIG] [-p PREFIX] [-b TEMP]\n";
            echo    "-c            Use a custom config";
            echo    "-p            Set a prefix"
            echo    "-b            Point to a temporary yed install to build a package with";
            exit 1;;
    esac
done

if ! [ -f install.options ]; then
    echo "install.sh: [!] Missing 'install.options'."
    exit 1
fi

source install.options

mkdir -p ${DESTDIR}${lib_dir} || exit 1
mkdir -p ${DESTDIR}${plug_dir} || exit 1
mkdir -p ${DESTDIR}${inc_dir}/yed || exit 1
mkdir -p ${DESTDIR}${bin_dir} || exit 1
mkdir -p ${DESTDIR}${share_dir}/yed || exit 1

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
${CC} src/yed.c -o ${DESTDIR}${lib_dir}/libyed.so.new ${LIB_C_FLAGS} ${cfg} || exit $?

if [ "${strip}x" = "yesx" ]; then
    echo "    stripped libyed.so"
    strip ${DESTDIR}${lib_dir}/libyed.so.new
fi

if [ $(uname) = "Darwin" ]; then
    install_name_tool -id $(apath "${DESTDIR}${lib_dir}/libyed.so") "${DESTDIR}${lib_dir}/libyed.so.new"
fi

mv ${DESTDIR}${lib_dir}/libyed.so.new ${DESTDIR}${lib_dir}/libyed.so || exit 1
if [ $(uname) = "Darwin" ] && [ -d "${DESTDIR}${lib_dir}/libyed.so.new.dSYM" ]; then
    rm -rf "${DESTDIR}${lib_dir}/libyed.so.dSYM" || exit 1
    mv "${DESTDIR}${lib_dir}/libyed.so.new.dSYM" "${DESTDIR}${lib_dir}/libyed.so.dSYM" || exit 1
    mv "${DESTDIR}${lib_dir}/libyed.so.dSYM/Contents/Resources/DWARF/libyed.so.new" "${DESTDIR}${lib_dir}/libyed.so.dSYM/Contents/Resources/DWARF/libyed.so" || exit 1
fi
echo "Installed 'libyed.so':             ${DESTDIR}${lib_dir}"

echo "Creating include directory.."
cp src/*.h ${DESTDIR}${inc_dir}/yed || exit 1
echo "Installed headers:                 ${DESTDIR}${inc_dir}/yed"

echo "Compiling the driver.."

${CC} src/yed_driver.c src/whereami.c -o ${DESTDIR}${bin_dir}/yed.new ${DRIVER_C_FLAGS} ${cfg} || exit $?

if [ "${strip}x" = "yesx" ]; then
    echo "    stripped yed"
    strip ${DESTDIR}${bin_dir}/yed.new
fi

mv ${DESTDIR}${bin_dir}/yed.new ${DESTDIR}${bin_dir}/yed || exit 1
if [ $(uname) = "Darwin" ] && [ -d "${DESTDIR}${bin_dir}/yed.new.dSYM" ]; then
    rm -rf "${DESTDIR}${bin_dir}/yed.dSYM" || exit 1
    mv "${DESTDIR}${bin_dir}/yed.new.dSYM" "${DESTDIR}${bin_dir}/yed.dSYM" || exit 1
    mv "${DESTDIR}${bin_dir}/yed.dSYM/Contents/Resources/DWARF/yed.new" "${DESTDIR}${bin_dir}/yed.dSYM/Contents/Resources/DWARF/yed" || exit 1
fi
echo "Installed 'yed':                   ${DESTDIR}${bin_dir}"

echo "Creating default configuration.."
cp -r share/* ${DESTDIR}${share_dir}/yed

if [ -z "${TEMP}" ]; then
        ${CC} ${share_dir}/yed/start/init.c -o ${share_dir}/yed/start/init.so $(${bin_dir}/yed --print-cflags) $(${bin_dir}/yed --print-ldflags) || exit 1
else
        sed 's|<yed/plugin.h>|"../../src/plugin.h"|' -i share/start/init.c
        ${CC} share/start/init.c -o ${DESTDIR}${share_dir}/yed/start/init.so $(${TEMP}/bin/yed --print-cflags) $(${TEMP}/bin/yed --print-ldflags) || exit 1
fi
echo "Installed share items:             ${DESTDIR}${share_dir}/yed"

echo "Compiling plugins.."
pids=()
plugs=()

cd ${DIR}/plugins

if [ -z "${TEMP}" ]; then
        PATH="${bin_dir}:${PATH}" 
else
        PATH="${TEMP}/bin:${PATH}" 
fi
for b in $(find . -name "build.sh" | sed "s#^\./##"); do
    cd ${DIR}/plugins/$(dirname $b)
    d=$(dirname $b)

    bash build.sh &

    pids+=($!)
    plugs+=($(dirname $b))
done

for (( i=0; i<${#pids[*]}; ++i)); do
    wait ${pids[$i]} || exit 1
    mkdir -p $(dirname ${DESTDIR}${plug_dir}/${plugs[$i]}) || exit 1
    mv ${DIR}/plugins/${plugs[$i]}/$(basename ${plugs[$i]}).so ${DESTDIR}${plug_dir}/${plugs[$i]}.so || exit 1
    if [ $(uname) = "Darwin" ] && [ -d ${DIR}/plugins/${plugs[$i]}/$(basename ${plugs[$i]}).so.dSYM ]; then
        if [ -d ${DESTDIR}${plug_dir}/${plugs[$i]}.so.dSYM ]; then
            rm -rf ${DESTDIR}${plug_dir}/${plugs[$i]}.so.dSYM
        fi
        mv ${DIR}/plugins/${plugs[$i]}/$(basename ${plugs[$i]}).so.dSYM ${DESTDIR}${plug_dir}/${plugs[$i]}.so.dSYM
    fi
done
echo "Installed plugins:                 ${DESTDIR}${plug_dir}"
