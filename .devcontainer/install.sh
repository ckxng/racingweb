#!/bin/sh
# tested on Ubuntu 20.04 x86_64
# execute this while already inside checked out git repo
# github coderspaces will execute this automatically

set -e

sudo apt update
sudo apt install -y git make cmake automake gcc g++ \
  gdb doxygen cppcheck clang clang-format clang-tidy llvm valgrind linux-tools-common \
  python3-pip \
 libboost-all-dev libgraphicsmagick-q16-3 libpq-dev libssl-dev libfcgi-dev unixodbc-dev libmysqlclient-dev
sudo pip install pre-commit cpplint

git submodule update --init --recursive

mkdir -p dep/wt/build; cd dep/wt/build
cmake ..
make
sudo make install
cd ../../..

sudo ldconfig

pre-commit install

cp -r ./dep/wt/resources ./docroot/
