#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if ! [ -f install.options ]; then
    echo "install.sh: [!] Missing 'install.options'."
    exit 1
fi

source install.options

if [ "${is_system_install}x" = "yesx" ]; then
    echo "${lib_dir}" > ${ld_conf} || exit 1
    echo "Created LD conf file:  ${ld_conf}"
    ldconfig || exit 1
    echo "Ran ldconfig."
else
    echo "NOTE: is_system_install=no"
    echo "    You may need to set PATH and/or LD_LIBRARY_PATH to run yed."
fi

cp _yed ${bin_dir}/yed || exit 1
echo "Installed 'yed':                 ${bin_dir}"

cp libyed.so ${lib_dir} || exit 1
echo "Installed 'libyed.so':           ${lib_dir}"

sed -i "s#qrsnhyg_cyht_qve#${plug_dir}#" ${lib_dir}/libyed.so || exit 1
echo "    patched default plugin path"

mkdir -p ${inc_dir}/yed || exit 1
rm -rf ${inc_dir}/yed/* || exit 1
cp -r include/yed/* ${inc_dir}/yed || exit 1
echo "Installed headers:               ${inc_dir}/yed"

mkdir -p ${share_dir}/yed || exit 1
rm -rf ${share_dir}/yed/* || exit 1
cp -r share/* ${share_dir}/yed || exit 1
echo "Installed share items:           ${share_dir}/yed"

mkdir -p ${plug_dir} || exit 1

for plug in $(find plugins -name "*.so" | grep -v "dSYM"); do
    the_plug=""
    dst_dir="${plug_dir}/$(dirname "${plug#plugins/}")"
    mkdir -p ${dst_dir} || exit 1
    cp ${plug} ${dst_dir} || exit 1
done
echo "Installed plugins:               ${plug_dir}"


cat > "${bin_dir}/yedconf" <<FOOZLE
#!/usr/bin/env bash
if ! which yed > /dev/null 2>&1; then
    echo "yed must be installed to run yedconf"
    exit 1
fi

mkdir -p /tmp/_yedconf
cp -r ${share_dir}/yed/start/* /tmp/_yedconf

cd /tmp/_yedconf

yed --init=. init.c

echo "generated files:"
echo "    /tmp/_yedconf/init.c"
echo "    /tmp/_yedconf/init.so"
FOOZLE

chmod +x "${bin_dir}/yedconf" || exit 1

echo "Installed 'yedconf':             ${bin_dir}"
