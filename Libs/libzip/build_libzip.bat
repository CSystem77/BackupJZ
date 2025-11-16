@echo off
REM Script pour compiler libzip avec CMake et Visual Studio
REM Ce script doit être exécuté depuis le répertoire Libs/libzip

echo ========================================
echo Compilation de libzip pour Windows
echo ========================================
echo.

REM Vérifier que CMake est installé
set CMAKE_CMD=
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set CMAKE_CMD=cmake
    echo CMake trouve dans le PATH
    goto :cmake_found
)

REM Essayer les emplacements communs
if exist "C:\Program Files\CMake\bin\cmake.exe" (
    set CMAKE_CMD="C:\Program Files\CMake\bin\cmake.exe"
    echo CMake trouve dans Program Files
    goto :cmake_found
)

if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" (
    set CMAKE_CMD="C:\Program Files (x86)\CMake\bin\cmake.exe"
    echo CMake trouve dans Program Files (x86)
    goto :cmake_found
)

REM CMake non trouvé
echo ========================================
echo ERREUR: CMake n'est pas trouve
echo ========================================
echo.
echo Options pour installer CMake:
echo 1. Via winget: winget install Kitware.CMake
echo 2. Telecharger depuis: https://cmake.org/download/
echo 3. Voir le fichier INSTALL_CMAKE.md pour plus de details
echo.
echo Si CMake est installe ailleurs, modifiez ce script
echo pour specifier le chemin complet vers cmake.exe
echo.
pause
exit /b 1

:cmake_found

echo [1/4] Creation du repertoire build...
if exist build (
    echo Le repertoire build existe deja. Suppression...
    rmdir /s /q build
)
mkdir build
cd build

echo [2/4] Configuration avec CMake...
echo.
echo Tentative de configuration avec options minimales...
echo Si ZLIB n'est pas trouve, voir INSTALL_ZLIB.md pour les instructions
echo.

%CMAKE_CMD% .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DENABLE_OPENSSL=OFF -DENABLE_BZIP2=OFF -DENABLE_LZMA=OFF -DENABLE_ZSTD=OFF
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo ERREUR: La configuration CMake a echoue
    echo ========================================
    echo.
    echo Probable cause: ZLIB n'est pas trouve
    echo.
    echo Solutions:
    echo 1. Installer vcpkg et utiliser build_libzip_with_vcpkg.bat
    echo 2. Installer ZLIB manuellement (voir INSTALL_ZLIB.md)
    echo 3. Utiliser une version precompilee de libzip
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

echo [4/4] Copie des fichiers necessaires...
REM Chercher le fichier zip.lib dans les sous-repertoires
set LIB_FOUND=0

if exist Release\zip.lib (
    echo Copie de Release\zip.lib vers lib\zip.lib
    copy Release\zip.lib ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)

if exist Debug\zip.lib (
    echo Copie de Debug\zip.lib vers lib\zip.lib
    copy Debug\zip.lib ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)

if exist lib\Release\zip.lib (
    echo Copie de lib\Release\zip.lib vers lib\zip.lib
    copy lib\Release\zip.lib ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)

if exist lib\Debug\zip.lib (
    echo Copie de lib\Debug\zip.lib vers lib\zip.lib
    copy lib\Debug\zip.lib ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)

REM Recherche recursive
for /r %%f in (zip.lib) do (
    echo Copie de %%f vers lib\zip.lib
    copy "%%f" ..\lib\zip.lib
    set LIB_FOUND=1
    goto :lib_copied
)

:lib_copied
if %LIB_FOUND%==0 (
    echo ATTENTION: zip.lib n'a pas ete trouve dans le repertoire build
    echo Veuillez le copier manuellement dans Libs\libzip\lib\
)

echo.
echo ========================================
echo Compilation terminee avec succes!
echo ========================================
echo.
echo Le fichier zip.lib devrait maintenant etre dans Libs\libzip\lib\
echo.
cd ..
pause

