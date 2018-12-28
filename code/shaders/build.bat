@echo off
SETLOCAL ENABLEEXTENSIONS

IF NOT "%2"=="nc" (
cls
echo shader build did cls!
)

pushd c:\developer\Marching_squares\code\shaders

IF "%1"=="r" ( 
  echo Compiling shaders, release mode...
  DEL /Q BasicVertexShader.cso
  DEL /Q BasicPixelShader.cso
  FXC /nologo /O2 /WX /T vs_4_0 /E "vMain" /Fo BasicVertexShader.cso BasicShader.hlsl
  FXC /nologo /O2 /WX /T ps_4_0 /E "pMain" /Fo BasicPixelShader.cso BasicShader.hlsl
) ELSE IF "%1"=="ar" (
  echo Compiling shaders and showing assembly but not producing output, release mode...
  FXC /nologo /O2 /WX /T vs_4_0 /E "vMain" BasicShader.hlsl
  FXC /nologo /O2 /WX /T ps_4_0 /E "pMain" BasicShader.hlsl
) ELSE IF "%1"=="ad" (
  echo Compiling shaders and showing assembly but not producing output, debug, mode...
  FXC /nologo /Od /Zi /WX /T vs_4_0 /E "vMain" BasicShader.hlsl
  FXC /nologo /Od /Zi /WX /T ps_4_0 /E "pMain" BasicShader.hlsl
) ELSE (
  echo Compiling shaders, debug mode...
  DEL /Q BasicVertexShader.cso
  DEL /Q BasicPixelShader.cso
  FXC /nologo /Od /Zi /WX /T vs_4_0 /E "vMain" /Fo BasicVertexShader.cso BasicShader.hlsl
  FXC /nologo /Od /Zi /WX /T ps_4_0 /E "pMain" /Fo BasicPixelShader.cso BasicShader.hlsl
)

popd