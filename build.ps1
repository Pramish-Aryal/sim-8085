<#
        Build script
#>
if(!(Test-Path("./build/")))
{
        New-Item -Itemtype Directory "build"
}

$source_name = "..\code\main.cpp"
$executable_name = "simu-8085.exe"

# $lib_path =
# $include_path =

$compiler_flags = "/nologo", "/EHsc", "/Zi", "/FC", "/std:c++14"
# $linker_flags =

# $libraries = 

Push-Location .\build

#cl /MD $source_name /Fe$executable_name $compiler_flags /I$include_path /link /LIBPATH:$lib_path $libraries $linker_flags /SUBSYSTEM:console
cl $source_name /Fe$executable_name $compiler_flags

Pop-Location

