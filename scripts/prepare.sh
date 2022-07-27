#!/bin/sh

DEFAULT_VERSION="8.6.3"
DEFAULT_LINUX_DISTR="ubuntu"
MY_PATH=$(dirname "$0")
TEMP_PATH="$MY_PATH"/../tmp
DXFC_PATH="$MY_PATH"/../thirdparty/dxFeedCApi

# Print help
print_help() {
  echo "This script is used to download the required version of dxFeed C-API."
  echo
  echo "Syntax: $0 [-d|l|c|C|h]"
  echo "options:"
  echo "d     dxFeed API version (default = $DEFAULT_VERSION)"
  echo "l     dxFeed API Linux distributive (i.e the libc version,"
  echo "      'ubuntu' - new, 'centos' - old, default = $DEFAULT_LINUX_DISTR)"
  echo "c     Clear the downloaded files"
  echo "C     Clear everything, including library files (lib and include)."
  echo "h     Print this Help."
  echo
}

clear() {
  rm -r "$MY_PATH"/../tmp
}

clear_all() {
  clear
  rm -r "$DXFC_PATH"
}

DXFC_VERSION=$DEFAULT_VERSION
DXFC_LINUX_DISTR=$DEFAULT_LINUX_DISTR

while getopts ":hd:l:cC" option; do
  case $option in
  h)
    print_help
    exit
    ;;

  c) # cleanup
    clear
    exit
    ;;

  C) # cleanup
    clear_all
    exit
    ;;

  d) # set version
    if [ "$OPTARG" != "" ]; then
      DXFC_VERSION=$OPTARG
    fi
    ;;

  l) # set linux dist
    if [ "$OPTARG" != "" ]; then
      DXFC_LINUX_DISTR=$OPTARG
    fi
    ;;

  \?) # Invalid option
    echo "Error: Invalid option"
    exit
    ;;
  esac
done

clear

echo "dxFeed C-API version = '$DXFC_VERSION', distr = '$DXFC_LINUX_DISTR'"
echo "Start loading the library"

distr="linux"
if [ "$DXFC_LINUX_DISTR" = "centos" ]; then
  distr='centos'
fi

mkdir -p "$TEMP_PATH"
mkdir -p "$DXFC_PATH"
wget -O "$TEMP_PATH"/dxfeed-c-api-"$DXFC_VERSION"-$distr-no-tls.zip https://github.com/dxFeed/dxfeed-c-api/releases/download/"$DXFC_VERSION"/dxfeed-c-api-"$DXFC_VERSION"-$distr-no-tls.zip
unzip "$TEMP_PATH"/dxfeed-c-api-"$DXFC_VERSION"-$distr-no-tls.zip -d "$TEMP_PATH"

DXFC_API_ACTUAL_DIST_PATH="$TEMP_PATH"/dxfeed-c-api-"$DXFC_VERSION"-no-tls/DXFeedAll-"$DXFC_VERSION"-x64-no-tls
mkdir -p "$DXFC_PATH"/lib
mkdir -p "$DXFC_PATH"/include
cp "$DXFC_API_ACTUAL_DIST_PATH"/bin/x64/*.so "$DXFC_PATH"/lib
cp "$DXFC_API_ACTUAL_DIST_PATH"/include/*.h "$DXFC_PATH"/include

clear
