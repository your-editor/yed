#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR

if ! [ -f install.options ]; then
    echo "install.sh: [!] Missing 'install.options'."
    exit 1
fi

source install.options

cp _yed ${bin_dir}/yed
echo "Installed 'yed':       ${bin_dir}"

cp libyed.so ${lib_dir}
echo "Installed 'libyed.so': ${lib_dir}"

mkdir -p ${inc_dir}/yed
rm -rf ${inc_dir}/yed/*
cp -r include/yed/* ${inc_dir}/yed
echo "Installed headers:     ${inc_dir}/yed"

mkdir -p ${share_dir}/yed
rm -rf ${share_dir}/yed/*
cp -r share/* ${share_dir}/yed
echo "Installed share items: ${share_dir}/yed"

mkdir -p ${plug_dir}

for plug in $(find plugins -name "*.so" | grep -v "dSYM"); do
    the_plug=""
    dst_dir="${plug_dir}/$(dirname "${plug#plugins/}")"
    mkdir -p ${dst_dir}
    cp ${plug} ${dst_dir}
done
echo "Installed plugins:     ${plug_dir}"


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

chmod +x "${bin_dir}/yedconf"

echo "Installed 'yedconf':   ${bin_dir}"
