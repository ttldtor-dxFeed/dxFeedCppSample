# dxFeedCppSample

## Prerequisites

Linux | MacOS:
- wget, unzip, gcc (or clang), ninja or make, cmake, pwsh (optional)

Windows:
- powershell, cmake, msbuild or/and Visual Studio 2015+ 

## Prepare libs

Linux | MacOS (x64):
```shell
./scripts/prepare.sh
```

Windows (x64) | Linux | MacOS (x64):
```shell
./scripts/prepare.ps1
```

Advanced usage:
```shell
./scripts/prepare.sh -v 8.6.2 -d windows
```

```shell
./scripts/prepare.ps1 -v 8.6.1 -d macosx
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