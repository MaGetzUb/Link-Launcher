@echo off

where /q cl 

if ERRORLEVEL 1 (
    echo "Couldn't find cl.exe, please run this batch via Development Command Prompt for Visual Studio, or set up the environment via 'vcvarsall.bat x64'"
    echo "On visual studio 2019, the vcvarsall.bat lies in 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build'"
) else (
    
    if not exist int\ (
        mkdir int
    )

    if not exist resources.res (
        rc resources.rc
    )

    cd int

    set COMPILER_FLAGS=/O2 /DNDEBUG
    set LINKER_FLAGS=/SUBSYSTEM:WINDOWS

    if "%1"=="-dbg" (
        set "COMPILER_FLAGS="
        set "LINKER_FLAGS=/SUBSYSTEM:CONSOLE"
    )

    echo "%COMPILER_FLAGS%"
    echo "%LINKER_FLAGS%"
     
    cl /EHsc /nologo ..\main.cpp /std:c++latest /utf-8 %COMPILER_FLAGS% /link %LINKER_FLAGS% kernel32.lib user32.lib shell32.lib ..\resources.res /out:..\LinkLauncher.exe

    cd ..

)