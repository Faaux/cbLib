@echo off

ctime -begin cbLib.ctm

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set ClangCompilerFlags= -c -x c++ -mthread-model posix -fmath-errno -D _DEBUG -D _WINDOWS -D _UNICODE -D UNICODE -g2 -gdwarf-2 -O0 -Wall -Werror  -std=c++14 -fcxx-exceptions -Wno-unused-function -Wno-unused-variable
set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7 /EHsc

set IncludeFolders= -I..\ThirdParty\include\ -I..\cbGame\include\ -I..\cbPlatform\include\

set CommonLinkerFlags= /NOLOGO /MANIFEST /DEBUG /MACHINE:X64 /INCREMENTAL:no /ERRORREPORT:PROMPT /LIBPATH:..\ThirdParty\lib\
set CommonLinkerLibs= "opengl32.lib" "glew32.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib"

IF NOT EXIST bin mkdir bin
pushd bin

del *.pdb > NUL 2> NUL

REM 64-bit build
REM Optimization switches /wO2
echo WAITING FOR PDB > lock.tmp

REM -------------- Working CLANG, doesnt end though for some reason -----------------------
REM clang %ClangCompilerFlags% %IncludeFolders% -D _USRDLL -D CBGAME_EXPORTS -D _WINDLL -D _MT -D _DLL  -o cbGame.obj ..\cbGame\src\cbGame.cpp
REM link %CommonLinkerFlags% %CommonLinkerLibs%  /DLL /NOENTRY /OUT:cbGame.dll /PDB:cbGame_%random%.pdb /IMPLIB:cbGame.lib cbGame.obj

REM clang %ClangCompilerFlags% %IncludeFolders% -o cbPlatform.obj ..\cbPlatform\src\cbPlatform.cpp
REM link %CommonLinkerFlags% %CommonLinkerLibs% /ENTRY:WinMain /SUBSYSTEM:WINDOWS /OUT:cbPlatform.exe /PDB:cbPlatform_%random%.pdb cbPlatform.obj
REM --------------                        END                       -----------------------


cl %CommonCompilerFlags% %IncludeFolders% ..\cbGame\src\cbGame.cpp -LD /link %CommonLinkerFlags%  %CommonLinkerLibs% -PDB:cbGame_%random%.pdb 
set LastError=%ERRORLEVEL%
del lock.tmp


cl %CommonCompilerFlags% %IncludeFolders% ..\cbPlatform\src\cbPlatform.cpp /link %CommonLinkerFlags% %CommonLinkerLibs%
popd

xcopy "ThirdParty\bin" ".\bin" /S /Y /q
xcopy ".\res" ".\bin" /S /Y /q

ctime -end cbLib.ctm %LastError%
pause
exit