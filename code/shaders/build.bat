@echo off
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION

IF NOT "%2"=="nc" (
cls
)

IF NOT EXIST ..\..\build\shaders mkdir ..\..\build\shaders
PUSHD ..\..\build\shaders

IF EXIST BasicVertexShader.cso DEL /Q BasicVertexShader.cso
IF EXIST BasicPixelShader.cso DEL /Q BasicPixelShader.cso

IF "%1"=="r" ( 
  echo Compiling shaders, release mode...

  FXC /nologo /O2 /WX /T vs_4_0 /E "vMain" /Fo BasicVertexShader.cso ..\..\code\shaders\BasicShader.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

  FXC /nologo /O2 /WX /T ps_4_0 /E "pMain" /Fo BasicPixelShader.cso ..\..\code\shaders\BasicShader.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

) ELSE IF "%1"=="ar" (
  echo Compiling shaders and showing assembly but not producing output, release mode...

  FXC /nologo /O2 /WX /T vs_4_0 /E "vMain" BasicShader.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

  FXC /nologo /O2 /WX /T ps_4_0 /E "pMain" BasicShader.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

) ELSE IF "%1"=="ad" (
  echo Compiling shaders and showing assembly but not producing output, debug mode...

  FXC /nologo /Od /Zi /WX /T vs_4_0 /E "vMain" BasicShader.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

  FXC /nologo /Od /Zi /WX /T ps_4_0 /E "pMain" BasicShader.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!
) ELSE (
  echo Compiling shaders, debug mode...

  FXC /nologo /Od /Zi /WX /T vs_4_0 /E "vMain" /Fo BasicVertexShader.cso ..\..\code\shaders\BasicShader.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

  FXC /nologo /Od /Zi /WX /T ps_4_0 /E "pMain" /Fo \BasicPixelShader.cso ..\..\code\shaders\BasicShader.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!
)

POPD