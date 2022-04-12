[![CI](https://github.com/ckxng/racingweb/actions/workflows/main.yml/badge.svg?branch=main)](https://github.com/ckxng/racingweb/actions/workflows/main.yml)

# Racing Web

Racing Web is an online tool for generating Perfect-N and Complimentary Perfect-N race schedules, and running the races
live from a computer or smartphone.

## Docs

See https://ckxng.github.io/racingweb/html/hierarchy.html

## Development

This project is built on [Wt](https://www.webtoolkit.eu/wt), which (at least to me) is somewhat difficult to manage in
Windows. Therefore, this project targets mainly Linux hosts. The primary target is Ubuntu 20.04 AMD64, though if other
architectures and target environments are feasible, pull requests to improve compatibility are always welcome.

    # build toolchain
    sudo apt install make cmake automake gcc g++

    # optional development tools
    sudo apt install gdb doxygen cppcheck clang clang-format clang-tidy llvm valgrind linux-tools-common
    sudo apt install python3-pip && sudo pip install pre-commit cpplint

    # download source
    git clone --recurse-submodules https://github.com/ckxng/racingweb.git
    cd racingweb
    pre-commit install # optional

    # witty and dependencies
    sudo apt install libboost-all-dev libgraphicsmagick-q16-3 libpq-dev libssl-dev libfcgi-dev
    cd dep/wt
    mkdir build; cd build
    cmake ..
    make
    make -C examples # optional
    sudo make install
    sudo ldconfig

## Copying

Copyright (c) 2022 [Cameron King](http://cameronking.me).  
Dual licensed under MIT and GPLv2 with OpenSSL exception.  
See LICENSE for details.

In short, I prefer more permissive licenses, so the code specifically related to this project is released under the MIT
license. This would allow any future developer to take portions of this project code and incorporate it into another
project under the MIT license terms. However, since [Wt](https://www.webtoolkit.eu/wt) has been released under the GPLv2
license with an OpenSSL exception, anyone who wishes to distribute binaries linked to the Wt libraries must do so under
the terms of the GPLv2.
