#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd ${DIR}

function corecount {
    getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu
}

source build.options
source install.options

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
export DRIVER_C_FLAGS="-rdynamic -ldl -lm -lpthread -Wl,${DTAGS},-rpath,${lib_dir}"

strnstr_test_prg="#include <string.h>\nint main() { strnstr(\"haystack\", \"needle\", 8); return 0; }"
if ! echo -e "${strnstr_test_prg}" | cc -Wall -x c -o /dev/null > /dev/null 2>&1 -; then
    LIB_C_FLAGS+=" -DNEED_STRNSTR"
fi

# Add this framework to the Mac debug build so
# that we can use Instruments.app to profile yed
if [ "$(uname)" == "Darwin" ]; then
    debug+=" -framework CoreFoundation"
fi

if [ "$#" -ge 1 ]; then
    cfg=${!1}
else
    cfg=${release}
fi

# I hate getting spammed with error messages
ERR_LIM=" -fmax-errors=3 -Wno-unused-command-line-argument"

${CC} --version ${ERR_LIM} 2>&1 > /dev/null

if [ $? -eq 0 ]; then
    cfg+=${ERR_LIM}
fi

export cfg=${cfg}

rm -rf build/*

echo "Compiling libyed.so.."
echo ${CC} ${LIB_C_FLAGS} ${cfg} src/yed.c -o lib/libyed.so || exit $?
${CC} ${LIB_C_FLAGS} ${cfg} src/yed.c -o build/libyed.so || exit $?
echo "Creating include directory.."
mkdir -p build/include/yed
cp src/*.h build/include/yed

echo "Compiling the driver.."
echo ${CC} ${DRIVER_C_FLAGS} ${cfg} src/yed_driver.c -o yed || exit $?
${CC} ${DRIVER_C_FLAGS} ${cfg} src/yed_driver.c -o build/yed || exit $?

if ! [ -d plugins ]; then
    echo "Grabbing plugins.."
    git clone git@github.com:kammerdienerb/yed-plugins plugins
fi

cd plugins
git pull

./install.sh run

echo "Done."
