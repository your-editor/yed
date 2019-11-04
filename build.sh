#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd ${DIR}

source build.options

LIB_C_FLAGS="-shared -fPIC -ldl -lm -lpthread"
DRIVER_C_FLAGS="-ldl -lm -lpthread"
PLUGIN_C_FLAGS="-shared -fPIC -Isrc -L. -lyed"

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

echo "Compiling libyed.so.."
# echo "${CC} src/yed.c ${LIB_C_FLAGS} ${cfg} -o libyed.so"
${CC} src/yed.c ${LIB_C_FLAGS} ${cfg} -o libyed.so

echo "Compiling the driver.."
# echo "${CC} src/yed_driver.c ${DRIVER_C_FLAGS} ${cfg} -o _yed"
${CC} src/yed_driver.c ${DRIVER_C_FLAGS} ${cfg} -o _yed &

# Compile all the plugins.
for plug in plugins/*.c; do
    echo "Compiling ${plug}.."
    # echo "${CC} ${plug} ${PLUGIN_C_FLAGS} ${cfg} -o "$(basename ${plug})".so"
    ${CC} ${plug} ${PLUGIN_C_FLAGS} ${cfg} -o plugins/"$(basename -s".c" ${plug})".so &
done

wait

echo "Installing plugins.."
./install.sh

echo "Done."
