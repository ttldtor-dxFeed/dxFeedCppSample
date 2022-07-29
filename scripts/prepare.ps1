#!/bin/pwsh

param(
    [Alias("d")][string]$version,
    [Alias("c")][switch]$clear,
    [Alias("C")][switch]$clearAll,
    [Alias("h")][switch]$help
)

$DEFAULT_VERSION="8.6.3"
#$DEFAULT_LINUX_DISTR="ubuntu"
$MY_PATH="$PSScriptRoot"
$TEMP_PATH="$MY_PATH" + "/../tmp"
$DXFC_PATH="$MY_PATH" + "/../thirdparty/dxFeedCApi"

if ($clearAll) {
    
}
elseif ($clear)
{

}

