#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd ${DIR}

function corecount {
    getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu
}

source build.options

export LIB_C_FLAGS="-shared -fPIC -ldl -lm -lpthread"
export DRIVER_C_FLAGS="-ldl -lm -lpthread"
export PLUGIN_C_FLAGS="-shared -fPIC -I${DIR}/include -L${DIR} -lyed"

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

make all -j$(corecount)
