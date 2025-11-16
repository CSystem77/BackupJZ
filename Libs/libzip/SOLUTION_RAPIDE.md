# Solution Rapide : Compiler libzip sans vcpkg

Si vous ne voulez pas installer Git et vcpkg, voici des alternatives plus simples.

## Option 1 : Télécharger libzip précompilé (LE PLUS RAPIDE)

1. **Chercher une version précompilée de libzip pour Windows** :
   - GitHub releases : https://github.com/nih-at/libzip/releases
   - vcpkg artifacts (si disponible)
   - Autres sources de binaires Windows

2. **Extraire les fichiers** :
   - Copier `zip.lib` dans `Libs\libzip\lib\`
   - Vérifier que `zip.h` et `zipconf.h` sont dans `Libs\libzip\lib\`

## Option 2 : Utiliser MinGW-w64 (Alternative à Visual Studio)

Si vous avez MinGW-w64 installé :

```cmd
cd Libs\libzip
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Option 3 : Installer Git rapidement

1. **Télécharger Git** : https://git-scm.com/download/win
2. **Installer** en cochant "Add Git to PATH"
3. **Redémarrer le terminal**
4. **Relancer** `install_vcpkg_and_zlib.bat`

## Option 4 : Compiler ZLIB manuellement puis libzip

### Étape 1 : Télécharger ZLIB

1. Aller sur : https://www.zlib.net/
2. Télécharger "zlib source code" (fichier .tar.gz)
3. Extraire dans un dossier, par exemple : `C:\Tools\zlib-1.3`

### Étape 2 : Compiler ZLIB avec Visual Studio

```cmd
cd C:\Tools\zlib-1.3
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Étape 3 : Compiler libzip avec ZLIB

```cmd
cd C:\Users\CSystem\Desktop\BackupJZ\Libs\libzip
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DZLIB_ROOT=C:\Tools\zlib-1.3\build
cmake --build . --config Release
copy Release\zip.lib ..\lib\zip.lib
```

## Option 5 : Utiliser Conan (si installé)

```cmd
conan install zlib/1.2.13@ --build=missing
conan install libzip/1.10.1@ --build=missing
```

## Recommandation

**Pour aller vite** : Cherchez une version précompilée de libzip pour Windows et copiez simplement `zip.lib` dans `Libs\libzip\lib\`.

**Pour une solution complète** : Installez Git (5 minutes), puis utilisez `install_vcpkg_and_zlib.bat` qui gère tout automatiquement.

