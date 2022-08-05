# dxFeedCppSample

## Prerequisites

Linux | MacOS:
- wget, unzip, gcc (or clang), ninja or make, cmake, pwsh (optional)

Windows:
- powershell, cmake, msbuild or/and Visual Studio 2015+ 

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
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

## Prepare project files for Visual Studio (2015 x64)

```shell
mkdir build
cd build
cmake -G "Visual Studio 14 2015" -A x64 -DCMAKE_BUILD_TYPE=Release ..
```