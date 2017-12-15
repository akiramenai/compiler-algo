#!/bin/bash

set -xeu

apt-get update -q;
apt-get install \
  software-properties-common \
  python-software-properties -y;

add-apt-repository ppa:ubuntu-toolchain-r/test -y;
apt-get update -q;
apt-get install \
  build-essential \
  git \
  wget \
  gcc-7 \
  g++-7 -y;
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 50;
update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-7 50;
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 50;
update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-7 50;


wget "https://cmake.org/files/v3.10/cmake-${cmake_ver}.tar.gz"
tar -xf cmake-${cmake_ver}.tar.gz
cd cmake-${cmake_ver}
./bootstrap --prefix=/usr/local > /dev/null
make install -j$(nproc) > /dev/null
cd ..

boost_ver2="${boost_ver//./_}"
wget "https://dl.bintray.com/boostorg/release/${boost_ver}/source/boost_${boost_ver2}.tar.gz"
tar -xf boost_${boost_ver2}.tar.gz
cd boost_${boost_ver2}
./bootstrap.sh --prefix=/usr --with-libraries=graph > /dev/null
./bjam install -j$(nproc) > /dev/null
cd ..
