# dxFeedCppSample
[![Build](https://github.com/ttldtor/dxFeedCppSample/actions/workflows/Build.yml/badge.svg)](https://github.com/ttldtor/dxFeedCppSample/actions/workflows/Build.yml)

## Prerequisites

Linux | MacOS:
- wget, unzip, gcc (or clang), ninja or make, cmake, pwsh (optional)

Windows:
- powershell, cmake, ninja, msbuild or/and Visual Studio 2015+ 

## Prepare libs

Linux | WSL:
```shell
./scripts/prepare.sh
```

```shell
./scripts/prepare.sh -v 8.6.2
```

```shell
./scripts/prepare.ps1 -d linux
```

```shell
./scripts/prepare.ps1 -v 8.6.3 -d linux
```

MacOS (x64):
```shell
./scripts/prepare.sh -d macosx
```

```shell
./scripts/prepare.ps1 -v 8.6.3  -d macosx
```

Windows:
```shell
./scripts/prepare.ps1
```

```shell
./scripts/prepare.ps1 -v 8.6.3
```

## Build

```shell
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

## Run

Linux | MacOS | WSL: 
```shell
chmod +x ./dxFeedCppSample
./dxFeedCppSample
```

Windows:
```shell
./dxFeedCppSample.exe
```

## Prepare project files for Visual Studio (2015 x64)

```shell
mkdir build
cd build
cmake -G "Visual Studio 14 2015" -A x64 -DCMAKE_BUILD_TYPE=Release ..
```

## TBD

- Automatic detection of the current operating system for the correct loading of the bundle with dxFeed C-API.
- Candle' doxygen comments
- Summary
- Profile
- Order\SpreadOrder
- TimeAndSale
- Greeks
- TheoPrice
- Underlying
- Series
- Configuration
- Other doxygen comments
- Sub multiplexing\caching
- Stream\Ticker buffer
- FOD\FOB
- Incremental
- Counter tool