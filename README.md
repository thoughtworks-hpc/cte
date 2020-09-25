# CTE: CDCF Trading Engine

![](https://github.com/thoughtworks-hpc/cte/workflows/Ubuntu%20MacOS%20Build/badge.svg)
![](https://github.com/thoughtworks-hpc/cte/workflows/Coding%20Style/badge.svg)

## Build from source

Requires:

- CMake 3.10 above
- conan 1.24 above

Checkout to project root and run following commands:

```shell
$ conan remote add inexorgame "https://api.bintray.com/conan/inexorgame/inexor-conan"
$ conan remote add hpc "https://api.bintray.com/conan/grandmango/cdcf"
$ mkdir build && cd build
$ conan install .. --build missing
$ cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_paths.cmake
$ cmake --build . -j
$ ctest .
```

## Related Project

[CDCF: C++ Distributed Computing Framework](https://github.com/thoughtworks-hpc/cdcf)

## Document
[CTE Develop Document](https://https://github.com/thoughtworks-hpc/cte/blob/develop/document/CTE%20Develop%20Document.pdf)