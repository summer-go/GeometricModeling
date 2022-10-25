# Usage:
```shell
cd \path\to\code\
git submodule update --init --recursive

#cmake
mkdir cxxbuild && cd cxxbuild && cmake ../ && make -j4

#webassembly
mkdir jsbuild && cd jsbuild && emcmake cmake ../ && make -j4
```