@echo off
REM Script pour compiler libzip avec vcpkg (gère automatiquement les dépendances)
REM Ce script utilise vcpkg pour installer ZLIB et autres dépendances

echo ========================================
echo Compilation de libzip avec vcpkg
echo ========================================
echo.

REM Vérifier vcpkg
set VCPKG_ROOT=C:\vcpkg
set VCPKG_CMD=

if exist "%VCPKG_ROOT%\vcpkg.exe" (
    set VCPKG_CMD=%VCPKG_ROOT%\vcpkg.exe
    goto :vcpkg_found
)

where vcpkg >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set VCPKG_CMD=vcpkg
    REM Trouver le répertoire vcpkg
    for /f "tokens=*" %%i in ('where vcpkg') do set VCPKG_CMD_PATH=%%i
    for %%i in ("%VCPKG_CMD_PATH%") do set VCPKG_ROOT=%%~dpi
    goto :vcpkg_found
)

echo ========================================
echo vcpkg n'est pas trouve
echo ========================================
echo.
echo Veuillez installer vcpkg d'abord:
echo   install_vcpkg_and_zlib.bat
echo.
echo Ou continuer sans vcpkg (peut echouer si ZLIB n'est pas installe)
echo.
pause
goto :standard_build

:vcpkg_found
echo vcpkg trouve: %VCPKG_CMD%
echo Installation de zlib via vcpkg...
echo Utilisation du mode classique pour eviter les conflits avec vcpkg.json...
%VCPKG_CMD% install zlib:x64-windows --classic
if %ERRORLEVEL% NEQ 0 (
    echo ERREUR: Installation de zlib a echoue
    pause
    exit /b 1
)

echo Configuration avec vcpkg toolchain...
if "%VCPKG_ROOT:~-1%"=="\" (
    set CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%scripts\buildsystems\vcpkg.cmake
) else (
    set CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
)
goto :configure

:standard_build
echo Utilisation de la methode standard (sans vcpkg)...
set CMAKE_TOOLCHAIN_FILE=

:configure
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
    rmdir /s /q build
)
mkdir build
cd build

echo [2/4] Configuration avec CMake...
if defined CMAKE_TOOLCHAIN_FILE (
    %CMAKE_CMD% .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE%
) else (
    %CMAKE_CMD% .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
)

if %ERRORLEVEL% NEQ 0 (
    echo ERREUR: La configuration CMake a echoue
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
if exist Release\zip.lib (
    copy Release\zip.lib ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)
if exist Debug\zip.lib (
    copy Debug\zip.lib ..\lib\zip.lib
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
)

echo.
echo Compilation terminee!
cd ..
pause

