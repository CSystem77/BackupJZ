@echo off
REM Script pour installer vcpkg et ZLIB automatiquement

echo ========================================
echo Installation de vcpkg et ZLIB
echo ========================================
echo.

REM Vérifier si Git est installé
where git >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ========================================
    echo ERREUR: Git n'est pas installe ou pas dans le PATH
    echo ========================================
    echo.
    echo Git est necessaire pour cloner vcpkg.
    echo.
    echo Options:
    echo 1. Installer Git depuis: https://git-scm.com/download/win
    echo    (Assurez-vous de cocher "Add Git to PATH" pendant l'installation)
    echo.
    echo 2. Installer ZLIB manuellement (voir INSTALL_ZLIB.md)
    echo.
    echo 3. Utiliser une version precompilee de libzip
    echo.
    echo Appuyez sur une touche pour quitter...
    pause >nul
    exit /b 1
)

REM Vérifier si vcpkg existe déjà
set VCPKG_DIR=C:\vcpkg
if exist "%VCPKG_DIR%\vcpkg.exe" (
    echo vcpkg existe deja dans %VCPKG_DIR%
    goto :install_zlib
)

echo [1/3] Clonage de vcpkg...
echo Cela peut prendre quelques minutes...
echo.

cd C:\
if not exist vcpkg (
    git clone https://github.com/microsoft/vcpkg.git
    if %ERRORLEVEL% NEQ 0 (
        echo ERREUR: Le clonage de vcpkg a echoue
        pause
        exit /b 1
    )
) else (
    echo Le dossier vcpkg existe deja
)

cd vcpkg

echo [2/3] Bootstrap de vcpkg...
.\bootstrap-vcpkg.bat
if %ERRORLEVEL% NEQ 0 (
    echo ERREUR: Le bootstrap de vcpkg a echoue
    pause
    exit /b 1
)

:install_zlib
echo [3/3] Installation de ZLIB...
echo Cela peut prendre quelques minutes...
echo.

REM Déterminer l'architecture (x64 par défaut)
set ARCH=x64-windows
if "%PROCESSOR_ARCHITECTURE%"=="x86" (
    set ARCH=x86-windows
)

echo Installation de ZLIB en mode classique...
"%VCPKG_DIR%\vcpkg.exe" install zlib:%ARCH% --classic
if %ERRORLEVEL% NEQ 0 (
    echo ERREUR: L'installation de ZLIB a echoue
    pause
    exit /b 1
)

echo.
echo ========================================
echo Installation terminee avec succes!
echo ========================================
echo.
echo vcpkg est installe dans: %VCPKG_DIR%
echo ZLIB est maintenant disponible
echo.
echo Vous pouvez maintenant compiler libzip avec:
echo   build_libzip_with_vcpkg.bat
echo.
pause

