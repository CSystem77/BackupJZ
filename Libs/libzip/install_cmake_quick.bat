@echo off
REM Script pour installer CMake rapidement via winget
echo ========================================
echo Installation de CMake via winget
echo ========================================
echo.

REM Vérifier si winget est disponible
where winget >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERREUR: winget n'est pas disponible
    echo.
    echo Veuillez installer CMake manuellement:
    echo 1. Aller sur https://cmake.org/download/
    echo 2. Telecharger "Windows x64 Installer"
    echo 3. Installer en cochant "Add CMake to system PATH"
    echo.
    echo Voir INSTALL_CMAKE.md pour plus de details
    pause
    exit /b 1
)

echo Installation de CMake en cours...
echo Cela peut prendre quelques minutes...
echo.

winget install Kitware.CMake

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Installation terminee avec succes!
    echo ========================================
    echo.
    echo IMPORTANT: Fermez et rouvrez ce terminal
    echo pour que le PATH soit mis a jour.
    echo.
    echo Ensuite, vous pourrez executer build_libzip.bat
    echo.
) else (
    echo.
    echo ERREUR: L'installation a echoue
    echo.
    echo Veuillez installer CMake manuellement:
    echo 1. Aller sur https://cmake.org/download/
    echo 2. Telecharger "Windows x64 Installer"
    echo 3. Installer en cochant "Add CMake to system PATH"
    echo.
)

pause

