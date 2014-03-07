@echo off

rem --- using: MS VC++ 2013 ---

set COMPILER=cl
set LINKER=link
rem for x64: maybe add /favor:AMD64 or /favor:INTEL64 to compiler options as appropriate
rem for x64: remove /arch:SSE from compiler options
:: set COMPILE_OPTIONS=/c /O2 /GL /arch:SSE /fp:fast /EHsc /GR- /GS- /MT /W4 /WL /nologo /Isrc
set COMPILE_OPTIONS=/EHsc /Foobj/
:: set COMPILE_OPTIONS=/EHsc /Foobj/ /O2 /fp:fast /GR- /GS- /MT /W4 /WL /nologo

mkdir obj
del /Q obj\*

:: For raytrace.exe
:: %COMPILER% %COMPILE_OPTIONS% raytrace.cpp imageio.cpp stb_image.c geometry.cpp camera.cpp noise.cpp

:: For pathtrace.exe
%COMPILER% %COMPILE_OPTIONS% isovoxel.cpp image.cpp stb_image.c

del /Q obj\*