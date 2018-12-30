@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION
cls

SET ReleaseMode=0
SET CompileShaders=0

IF "%1"=="r" SET ReleaseMode=1
IF "%2"=="r" SET ReleaseMode=1
IF "%1"=="s" SET CompileShaders=1
IF "%2"=="s" SET CompileShaders=1

IF %CompileShaders%==1 (
  IF %ReleaseMode%==1 (
    call shaders\build.bat r nc
  ) ELSE (
    call shaders\build.bat d nc
  )
)

IF %errorlevel% NEQ 0 EXIT /b %errorlevel%


REM ----------------------------------------------------------------------------------
REM Compiler Options
REM https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=vs-2017

IF %ReleaseMode%==1 (
  ECHO Release mode
  SET CompilerOptions=/WL /O2 /nologo /fp:fast /fp:except- /EHsc /Gm- /Zo /Oi /WX /W4 /wd4100 /wd4201 /FC /Z7 /GS-
  SET CompilerOptions=/Gs9999999 /DRELEASE=1 /DWIN32=1 !CompilerOptions!
) ELSE (
  ECHO Debug mode
  SET CompilerOptions=/WL /Od /nologo /fp:fast /fp:except- /EHsc /Gm- /Zo /Oi /WX /W4 /wd4100 /wd4201 /FC /Z7 /GS-
  SET CompilerOptions=/Gs9999999 /DDEBUG=1 /DWIN32=1 !CompilerOptions!
)


REM WL		 One line diagonostics
REM Ox		 Code generation x E [d = Debug, 1 = small code, 2 = fast code]
REM fp:fast    Fast floating point code generated
REM fp:except- No floating point exceptions
REM EHsc       Catches C++ exceptions only
REM GM-		Enables minimal rebuild (- disables it, we want all files compiled all the time)
REM Zo		 Generate enhanced debugging information for optimized code in non-debug builds
REM Oi		 Generates intrinsic functions.
REM WX		 Treats all warnings as errors
REM W4		 All warnings
REM wx		 Except...
REM			  4100 'identifier': unreferenced formal parameter
REM			  4201 nonstandard extension used: nameless struct/union
REM FC		 Display full path of source code files passed to cl.exe in diagnostic text
REM Zi		 Produces separate PDB file that contains all the symbolic debugging information for use with the debugger
REM GS		 Buffer security check
REM Gs		 Control stack checking calls



REM ----------------------------------------------------------------------------------
REM Linker Options
REM https://docs.microsoft.com/en-us/cpp/build/reference/linker-options?view=vs-2017

set LinkerOptions=/MANIFEST:NO /INCREMENTAL:NO /OPT:REF user32.lib d3d10.lib dxgi.lib dxguid.lib
REM gdi32.lib winmm.lib kernel32.lib

REM incremental:no Link Incrementally is not selected
REM opt:ref		eliminates functions and data that are never referenced



REM ----------------------------------------------------------------------------------
REM Build

IF NOT EXIST c:\developer\Marching_squares\build mkdir c:\developer\Marching_squares\build
pushd c:\developer\Marching_squares\build

ECHO Removing all old files...
del /Q *.*

ECHO Building...
cl %CompilerOptions% ../code/*.cpp /link /SUBSYSTEM:windows %LinkerOptions% /out:Marching_squares.exe

IF %errorlevel% NEQ 0 (
  popd
  EXIT /b %errorlevel%
)

echo Embedding manifest...
mt.exe -nologo -manifest "../data/a.manifest" -outputresource:"Marching_squares.exe;#1"

IF %errorlevel% NEQ 0 (
  popd
  EXIT /b %errorlevel%
)

ECHO All done.
popd