# dxFeedCppSample

## Prerequisites

Linux:
- wget, unzip, gcc (or clang), ninja or make, cmake

Windows:
- cmake, msbuild or/and Visual Studio 2015+, [vcredist2015](https://www.microsoft.com/en-us/download/details.aspx?id=52685) (for running) 

## Prepare libs

Linux:
```shell
./scripts/prepare.sh
```

Windows:
```shell
./scripts/prepare.ps1
```

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

## Prepare project files for Visual Studio (2015 x64)

```shell
mkdir build
cd build
cmake -G "Visual Studio 14 2015" -A x64 -DCMAKE_BUILD_TYPE=Release ..
```