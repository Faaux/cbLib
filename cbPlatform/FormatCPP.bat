@ECHO off
for /f %%f in ('dir /b %cd%\src') do clang-format -style=file -i %~dp0src\%%f
for /f %%f in ('dir /b %cd%\include') do clang-format -style=file -i %~dp0include\%%f