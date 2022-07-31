#!/bin/sh

DEFAULT_VERSION="8.6.3"
DEFAULT_DISTR="ubuntu"
MY_PATH=$(dirname "$0")
TEMP_PATH="$MY_PATH"/../tmp
DXFC_PATH="$MY_PATH"/../thirdparty/dxFeedCApi
DXFC_LIB_PATH="$DXFC_PATH"/lib
DXFC_INCLUDE_PATH="$DXFC_PATH"/include

# Print help
print_help() {
  echo "This script is used to download the required version of dxFeed C-API."
  echo
  echo "Syntax: $0 [-v|l|c|x|h]"
  echo "options:"
  echo "v     dxFeed API version (default = $DEFAULT_VERSION)"
  echo "d     dxFeed API distributive (possible values: 'centos' (old libc), "
  echo "      'ubuntu' (new libc), 'windows', 'macosx', default = $DEFAULT_DISTR)"
  echo "c     Clear the downloaded files"
  echo "x     Clear everything, including library files (lib and include)."
  echo "h     Print this Help."
  echo
}

clear() {
  rm -r "$TEMP_PATH"
}

clear_all() {
  clear
  rm -r "$DXFC_PATH"
}

DXFC_VERSION=$DEFAULT_VERSION
DXFC_DISTR=$DEFAULT_DISTR

while getopts ":hv:d:cx" option; do
  case $option in
  h)
    print_help
    exit
    ;;

  c)
    clear
    exit
    ;;

  x)
    clear_all
    exit
    ;;

  v) # set version
    if [ "$OPTARG" != "" ]; then
      DXFC_VERSION=$OPTARG
    fi
    ;;

  d) # set distributive
    if [ "$OPTARG" != "" ]; then
      DXFC_DISTR=$OPTARG
    fi
    ;;

  \?) # Invalid option
    echo "Error: Invalid option"
    exit
    ;;
  esac
done

clear_all

DISTR="linux"
if [ "$DXFC_DISTR" = "centos" ]; then
  DISTR='centos'
elif [ "$DXFC_DISTR" = "windows" ] || [ "$DXFC_DISTR" = "win" ]; then
  DISTR='windows'
elif [ "$DXFC_DISTR" = "macosx" ] || [ "$DXFC_DISTR" = "macos" ] || [ "$DXFC_DISTR" = "mac" ]; then
  DISTR='macosx'
fi

echo "dxFeed C-API version = '$DXFC_VERSION', distr = '$DISTR'"
echo "Start loading the library"

mkdir -p "$TEMP_PATH"
mkdir -p "$DXFC_PATH"
mkdir -p "$DXFC_LIB_PATH"
mkdir -p "$DXFC_INCLUDE_PATH"

ZIP_FILE="$TEMP_PATH"/dxfeed-c-api-"$DXFC_VERSION"-$DISTR-no-tls.zip
URL=https://github.com/dxFeed/dxfeed-c-api/releases/download/"$DXFC_VERSION"/dxfeed-c-api-"$DXFC_VERSION"-$DISTR-no-tls.zip

wget -O "$ZIP_FILE" "$URL"
unzip "$ZIP_FILE" -d "$TEMP_PATH"

DXFC_API_SOURCE_PATH="$TEMP_PATH"/dxfeed-c-api-"$DXFC_VERSION"-no-tls/DXFeedAll-"$DXFC_VERSION"-x64-no-tls
if [ "$DISTR" = "centos" ] || [ "$DISTR" = "linux" ]; then
  cp "$DXFC_API_SOURCE_PATH"/bin/x64/*.so "$DXFC_LIB_PATH"
elif [ "$DISTR" = "windows" ]; then
  DXFC_API_SOURCE_PATH="$TEMP_PATH"/dxfeed-c-api-"$DXFC_VERSION"-no-tls
  cp "$DXFC_API_SOURCE_PATH"/bin/x64/*.dll "$DXFC_LIB_PATH"
  cp "$DXFC_API_SOURCE_PATH"/bin/x64/*.lib "$DXFC_LIB_PATH"
  cp "$DXFC_API_SOURCE_PATH"/bin/x64/*.pdb "$DXFC_LIB_PATH"
elif [ "$DISTR" = "macosx" ]; then
  cp "$DXFC_API_SOURCE_PATH"/bin/x64/*.dylib "$DXFC_LIB_PATH"
fi

cp "$DXFC_API_SOURCE_PATH"/include/*.h "$DXFC_INCLUDE_PATH"

clear
