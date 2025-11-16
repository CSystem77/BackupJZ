@echo off
REM Script pour compiler libzip avec seulement zlib (sans bzip2, lzma, zstd)
REM Ce script compile une version minimale de libzip

echo ========================================
echo Compilation de libzip (version minimale)
echo ========================================
echo.
echo Cette version n'inclut que le support zlib
echo (sans bzip2, lzma, zstd)
echo.

REM Vérifier CMake
set CMAKE_CMD=
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set CMAKE_CMD=cmake
    goto :cmake_found
)

if exist "C:\Program Files\CMake\bin\cmake.exe" (
    set CMAKE_CMD="C:\Program Files\CMake\bin\cmake.exe"
    goto :cmake_found
)

echo ERREUR: CMake n'est pas trouve
pause
exit /b 1

:cmake_found
echo [1/4] Creation du repertoire build...
if exist build (
    echo Suppression de l'ancien build...
    rmdir /s /q build
)
mkdir build
cd build

echo [2/4] Configuration avec CMake (version minimale)...
echo Desactivation de bzip2, lzma, zstd...
%CMAKE_CMD% .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DENABLE_BZIP2=OFF -DENABLE_LZMA=OFF -DENABLE_ZSTD=OFF -DENABLE_OPENSSL=OFF
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo ERREUR: La configuration CMake a echoue
    echo ========================================
    echo.
    echo Verifiez que zlib est disponible
    echo.
    cd ..
    pause
    exit /b 1
)

echo [3/4] Compilation...
%CMAKE_CMD% --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo ERREUR: La compilation a echoue
    cd ..
    pause
    exit /b 1
)

echo [4/4] Copie des fichiers...
set LIB_FOUND=0
if exist lib\Release\zip.lib (
    copy lib\Release\zip.lib ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)
if exist Release\zip.lib (
    copy Release\zip.lib ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)
for /r %%f in (zip.lib) do (
    copy "%%f" ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)

:lib_copied
if %LIB_FOUND%==0 (
    echo ATTENTION: zip.lib non trouve
) else (
    echo zip.lib copie avec succes!
)

echo.
echo ========================================
echo Compilation terminee!
echo ========================================
echo.
echo Le fichier zip.lib est dans Libs\libzip\lib\
echo Cette version ne supporte que zlib (pas bzip2, lzma, zstd)
echo.
cd ..
pause

