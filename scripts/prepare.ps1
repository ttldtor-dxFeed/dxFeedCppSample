#!/bin/pwsh

<#
.SYNOPSIS

Prepares the dxFeed C-API library for use when building the program

.DESCRIPTION

.PARAMETER Version
dxFeed API version (pattern: ^\d+\.\d+\.\d+$, default = 8.6.3)

.PARAMETER Distributive
dxFeed API distributive (possible values: "linux", "ubuntu" (new libc), "windows", "win", "centos" (old libc), "macosx", "macos", "mac"; default = "windows")

.PARAMETER Clear
Clear the downloaded files

.PARAMETER ClearAll
Clear everything, including library files (lib and include).

.EXAMPLE
./prepare.ps1

.EXAMPLE
./prepare.ps1 -v 8.6.3 -d macosx

.EXAMPLE
./prepare.ps1 -v 8.6.2 -d centos

#>
    [CmdletBinding()]
param(
    [Parameter(HelpMessage = "dxFeed API version")]
    [ValidatePattern("^\d+\.\d+\.\d+$")]
    [Alias("v")][string]$Version,
    [Parameter(HelpMessage = "dxFeed API distributive")]
    [ValidatePattern("^linux|ubuntu|windows|win|centos|macosx|macos|mac$")]
    [Alias("d")][string]$Distributive,
    [Parameter(HelpMessage = "Clear the downloaded files")][Alias("c")][switch]$Clear,
    [Parameter(HelpMessage = "Clear everything, including library files (lib and include).")][Alias("x")][switch]$clearAll
)

$DEFAULT_VERSION = "8.6.3"
$DEFAULT_DISTR = "windows"
$MY_PATH = "$PSScriptRoot"
$TEMP_PATH = "$MY_PATH" + "/../tmp"
$DXFC_PATH = "$MY_PATH" + "/../thirdparty/dxFeedCApi"
$DXFC_LIB_PATH = "$DXFC_PATH" + "/lib"
$DXFC_INCLUDE_PATH = "$DXFC_PATH" + "/include"

function Clear-Temp()
{
    if (Test-Path "$TEMP_PATH")
    {
        Remove-Item -Recurse -Force "$TEMP_PATH"
    }
}

function Clear-All()
{
    Clear-Temp

    if (Test-Path "$DXFC_PATH")
    {
        Remove-Item -Recurse -Force "$DXFC_PATH"
    }
}


if ($clearAll)
{
    Clear-All

    exit
}
elseif ($Clear)
{
    Clear-Temp

    exit
}

Clear-All

$DXFC_VERSION = $DEFAULT_VERSION

if ($Version)
{
    $DXFC_VERSION = $Version
}

$DISTR = $DEFAULT_DISTR

if (($Distributive -eq "linux") -or ($Distributive -eq "ubuntu"))
{
    $DISTR = "linux"
}
elseif ($Distributive -eq "centos")
{
    $DISTR = "centos"
}
elseif (($Distributive -eq "macosx") -or ($Distributive -eq "macos") -or ($Distributive -eq "mac"))
{
    $DISTR = 'macosx'
}

Write-Host "dxFeed C-API version = '$DXFC_VERSION', distr = '$DISTR'"
Write-Host "Start loading the library"

New-Item -ItemType Directory -Force -Path "$TEMP_PATH" | Out-Null
New-Item -ItemType Directory -Force -Path "$DXFC_PATH" | Out-Null
New-Item -ItemType Directory -Force -Path "$DXFC_LIB_PATH" | Out-Null
New-Item -ItemType Directory -Force -Path "$DXFC_INCLUDE_PATH" | Out-Null

$ZIP_FILE = "$TEMP_PATH" + "/dxfeed-c-api-" + $DXFC_VERSION + "-" + "$DISTR" + "-no-tls.zip"
$URL = "https://github.com/dxFeed/dxfeed-c-api/releases/download/" + $DXFC_VERSION + "/dxfeed-c-api-" + $DXFC_VERSION + "-" + $DISTR + "-no-tls.zip"
Invoke-WebRequest -Uri "$URL" -OutFile "$ZIP_FILE"
Expand-Archive "$ZIP_FILE" -DestinationPath "$TEMP_PATH" -Force

$DXFC_API_SOURCE_PATH = "$TEMP_PATH" + "/dxfeed-c-api-" + $DXFC_VERSION + "-no-tls/DXFeedAll-" + $DXFC_VERSION + "-x64-no-tls"
$DXFC_API_SOURCE_BIN_PATH = "$DXFC_API_SOURCE_PATH" + "/bin/x64/*"

if (($DISTR -eq "centos") -or ($DISTR -eq "linux"))
{
    Copy-Item -Path "$DXFC_API_SOURCE_BIN_PATH" -Include *.so -Recurse -Container:$false -Destination "$DXFC_LIB_PATH"
}
elseif ($DISTR -eq "windows")
{
    $DXFC_API_SOURCE_PATH = "$TEMP_PATH" + "/dxfeed-c-api-" + $DXFC_VERSION + "-no-tls"
    $DXFC_API_SOURCE_BIN_PATH = "$DXFC_API_SOURCE_PATH" + "/bin/x64/*"
    Copy-Item -Path "$DXFC_API_SOURCE_BIN_PATH" -Include *.dll -Recurse -Container:$false -Destination "$DXFC_LIB_PATH"
    Copy-Item -Path "$DXFC_API_SOURCE_BIN_PATH" -Include *.lib -Recurse -Container:$false -Destination "$DXFC_LIB_PATH"
    Copy-Item -Path "$DXFC_API_SOURCE_BIN_PATH" -Include *.pdb -Recurse -Container:$false -Destination "$DXFC_LIB_PATH"
}
elseif ($DISTR -eq "macosx" )
{
    Copy-Item -Path "$DXFC_API_SOURCE_BIN_PATH" -Include *.dylib -Recurse -Container:$false -Destination "$DXFC_LIB_PATH"
}

$DXFC_API_SOURCE_INCLUDE_PATH = "$DXFC_API_SOURCE_PATH" + "/include/*"
Copy-Item -Path "$DXFC_API_SOURCE_INCLUDE_PATH" -Container:$false -Recurse -Destination "$DXFC_INCLUDE_PATH"

Clear-Temp