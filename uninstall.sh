#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if ! [ -f install.options ]; then
    echo "uninstall.sh: [!] Missing 'install.options'."
    exit 1
fi

source install.options

function uninstall {
    yes | rm ${bin_dir}/yed
    echo "Uninstalled ${bin_dir}/yed"
    yes | rm ${lib_dir}/libyed.so
    echo "Uninstalled ${lib_dir}/libyed.so"
    yes | rm -rf ${inc_dir}/yed
    echo "Uninstalled ${inc_dir}/yed"
    yes | rm -rf ${plug_dir}
    echo "Uninstalled ${plug_dir}"
}

function confirm {
    echo "The following files/directories will be removed:
${bin_dir}/yed
${lib_dir}/libyed.so
${inc_dir}/yed
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
