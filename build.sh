#!/bin/sh

cd sqlite/
tar xf sqlite-autoconf-3240000.tar.gz
cd sqlite-autoconf-3240000/

mkdir out/
install_dir=$PWD/out/
echo $install_dir

#./configure --prefix=$install_dir
./configure --prefix=$install_dir --host=mips-linux-gnu

make
make install
