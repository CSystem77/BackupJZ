@echo off
REM Script pour installer les dépendances manquantes de libzip via vcpkg

echo ========================================
echo Installation des dependances libzip
echo ========================================
echo.

REM Vérifier vcpkg
set VCPKG_CMD=
if exist "C:\vcpkg\vcpkg.exe" (
    set VCPKG_CMD=C:\vcpkg\vcpkg.exe
    goto :vcpkg_found
)

where vcpkg >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set VCPKG_CMD=vcpkg
    goto :vcpkg_found
)

echo ERREUR: vcpkg n'est pas trouve
echo Veuillez installer vcpkg d'abord avec install_vcpkg_and_zlib.bat
pause
exit /b 1

:vcpkg_found
echo vcpkg trouve: %VCPKG_CMD%
echo.
echo Installation des dependances (cela peut prendre plusieurs minutes)...
echo.

echo [1/3] Installation de bzip2...
%VCPKG_CMD% install bzip2:x64-windows --classic
if %ERRORLEVEL% NEQ 0 (
    echo ATTENTION: Installation de bzip2 a echoue
)

echo.
echo [2/3] Installation de liblzma...
%VCPKG_CMD% install liblzma:x64-windows --classic
if %ERRORLEVEL% NEQ 0 (
    echo ATTENTION: Installation de liblzma a echoue
)

echo.
echo [3/3] Installation de zstd...
%VCPKG_CMD% install zstd:x64-windows --classic
if %ERRORLEVEL% NEQ 0 (
    echo ATTENTION: Installation de zstd a echoue
)

echo.
echo ========================================
echo Installation terminee!
echo ========================================
echo.
echo Les bibliotheques sont maintenant dans:
echo C:\vcpkg\installed\x64-windows\lib
echo.
echo Vous pouvez maintenant recompiler votre projet BackupJZ
echo.
pause

