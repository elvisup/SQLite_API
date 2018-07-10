#!/bin/sh

cd sqlite/sqlite-autoconf-3240000/

mkdir out/
install_dir=$PWD/out/
echo $install_dir

./configure --prefix=$install_dir

make
make install
