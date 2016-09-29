@ECHO off
echo "Formatting cbPlatform"
for /f %%f in ('dir /b %cd%cbPlatform\src') do clang-format -style=file -i %~dp0cbPlatform\src\%%f
for /f %%f in ('dir /b %cd%cbPlatform\include') do clang-format -style=file -i %~dp0cbPlatform\include\%%f

echo "Formatting cbGame"
for /f %%f in ('dir /b %cd%cbGame\src') do clang-format -style=file -i %~dp0cbGame\src\%%f
for /f %%f in ('dir /b %cd%cbGame\include') do clang-format -style=file -i %~dp0cbGame\include\%%f