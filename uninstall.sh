#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if ! [ -f install.options ]; then
    echo "uninstall.sh: [!] Missing 'install.options'."
    exit 1
fi

source install.options

function uninstall {
    yes | rm ${bin_dir}/yed || exit 1
    echo "Uninstalled ${bin_dir}/yed"
    yes | rm ${bin_dir}/yedconf || exit 1
    echo "Uninstalled ${bin_dir}/yedconf"
    yes | rm ${lib_dir}/libyed.so || exit 1
    echo "Uninstalled ${lib_dir}/libyed.so"
    yes | rm -rf ${inc_dir}/yed || exit 1
    echo "Uninstalled ${inc_dir}/yed"
    yes | rm -rf ${share_dir}/yed || exit 1
    echo "Uninstalled ${share_dir}/yed"
    yes | rm -rf ${plug_dir} || exit 1
    echo "Uninstalled ${plug_dir}"

    if [ "${is_system_install}x" = "yesx" ]; then
        yes | rm ${ld_conf} || exit 1
        echo "Uninstalled ${ld_conf}"
        ldconfig || exit 1
        echo "Ran ldconfig"
    fi
}

function confirm {
    if [ "${is_system_install}x" = "yesx" ]; then
        echo "The following files/directories will be removed:
${bin_dir}/yed
${bin_dir}/yedconf
${lib_dir}/libyed.so
${inc_dir}/yed
${share_dir}/yed
${plug_dir}
${ld_conf}
"
    else
        echo "The following files/directories will be removed:
${bin_dir}/yed
${bin_dir}/yedconf
${lib_dir}/libyed.so
${inc_dir}/yed
${share_dir}/yed
${plug_dir}
"
    fi

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
