# Installation de ZLIB pour libzip

ZLIB est une dépendance requise pour compiler libzip. Voici plusieurs méthodes pour l'installer.

## Méthode 1 : Via vcpkg (Recommandé - Gère automatiquement les dépendances)

### Installation de vcpkg

1. **Cloner vcpkg** :
   ```cmd
   cd C:\
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ```

2. **Bootstrap vcpkg** :
   ```cmd
   .\bootstrap-vcpkg.bat
   ```

3. **Installer ZLIB** :
   ```cmd
   .\vcpkg install zlib:x64-windows
   ```
   (ou `zlib:x86-windows` pour 32 bits)

4. **Compiler libzip avec vcpkg** :
   ```cmd
   cd C:\Users\CSystem\Desktop\BackupJZ\Libs\libzip
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
   cmake --build . --config Release
   ```

## Méthode 2 : Télécharger ZLIB précompilé

1. **Télécharger ZLIB pour Windows** :
   - Aller sur : https://www.zlib.net/
   - Télécharger la version Windows (ou utiliser vcpkg)

2. **Extraire et configurer** :
   - Extraire dans un dossier, par exemple : `C:\Tools\zlib`
   - Noter les chemins vers `zlib.lib` et le dossier `include`

3. **Compiler libzip avec ZLIB** :
   ```cmd
   cd Libs\libzip
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DZLIB_ROOT=C:\Tools\zlib
   cmake --build . --config Release
   ```

## Méthode 3 : Compiler ZLIB depuis les sources

1. **Télécharger les sources ZLIB** :
   - Aller sur : https://www.zlib.net/
   - Télécharger les sources

2. **Compiler ZLIB** :
   ```cmd
   cd zlib-1.3
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```

3. **Compiler libzip** :
   ```cmd
   cd C:\Users\CSystem\Desktop\BackupJZ\Libs\libzip
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DZLIB_ROOT=C:\chemin\vers\zlib\build
   cmake --build . --config Release
   ```

## Méthode 4 : Utiliser Conan (Gestionnaire de paquets C++)

Si vous utilisez Conan :

```cmd
conan install zlib/1.2.13@ --build=missing
```

## Solution rapide : Désactiver les fonctionnalités optionnelles

Si vous n'avez pas besoin de toutes les fonctionnalités, vous pouvez essayer de compiler avec des options minimales :

```cmd
cd Libs\libzip
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DENABLE_OPENSSL=OFF -DENABLE_BZIP2=OFF -DENABLE_LZMA=OFF -DENABLE_ZSTD=OFF
```

**Note** : ZLIB reste obligatoire, cette méthode ne résout pas le problème.

## Recommandation

**Utilisez vcpkg** - C'est la méthode la plus simple et la plus fiable pour gérer les dépendances sur Windows. Une fois vcpkg installé, il gère automatiquement ZLIB et toutes les autres dépendances.

