#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if [ "$(uname)" == "Darwin" ]; then
    function realpath() {
        [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
    }
fi

while getopts "c:p:" flag
do
    case "${flag}" in
        p) prefix=$(realpath ${OPTARG});;
        \?) echo "usage: $0 [-p PREFIX]" && exit 1;;
    esac
done

if ! [ -f install.options ]; then
    echo "install.sh: [!] Missing 'install.options'."
    exit 1
fi

source install.options


function uninstall {
    yes | rm ${bin_dir}/yed || exit 1
    echo "Uninstalled ${bin_dir}/yed"
    yes | rm -rf ${lib_dir}/libyed.so ${lib_dir}/libyed.so.dSYM || exit 1
    echo "Uninstalled ${lib_dir}/libyed.so"
    yes | rm -rf ${inc_dir}/yed || exit 1
    echo "Uninstalled ${inc_dir}/yed"
    yes | rm -rf ${share_dir}/yed || exit 1
    echo "Uninstalled ${share_dir}/yed"
    yes | rm -rf ${plug_dir} || exit 1
    echo "Uninstalled ${plug_dir}"
}

function confirm {
    echo "The following files/directories will be removed:
${bin_dir}/yed
${lib_dir}/libyed.so
${inc_dir}/yed
${share_dir}/yed
${plug_dir}
"

    # call with a prompt string or use a default
    read -r -p "${1:-Are you sure? [y/N]} " response
    case "$response" in
        [yY][eE][sS]|[yY])
            true
            ;;
        *)
            false
            ;;
    esac
}

if confirm; then
    uninstall
else
    echo "Uninstall cancelled."
fi
