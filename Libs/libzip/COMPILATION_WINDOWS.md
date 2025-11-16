# Guide de compilation de libzip pour Windows

Ce guide explique comment compiler libzip pour Windows avec Visual Studio.

## Prérequis

1. **CMake** (version 3.10 ou supérieure)
   - Télécharger depuis : https://cmake.org/download/
   - Ou installer via : `winget install Kitware.CMake`

2. **Visual Studio** avec les outils C++
   - Visual Studio 2019 ou 2022 avec "Desktop development with C++"

## Méthode 1 : Script automatique (Recommandé)

1. Ouvrir un terminal (PowerShell ou CMD) en tant qu'administrateur
2. Naviguer vers le répertoire libzip :
   ```cmd
   cd Libs\libzip
   ```
3. Exécuter le script :
   ```cmd
   build_libzip.bat
   ```

Le script va :
- Créer un répertoire `build`
- Configurer le projet avec CMake
- Compiler la bibliothèque en mode Release
- Copier `zip.lib` dans le répertoire `lib/`

## Méthode 2 : Compilation manuelle

### Étape 1 : Créer le répertoire build

```cmd
cd Libs\libzip
mkdir build
cd build
```

### Étape 2 : Configurer avec CMake

```cmd
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
```

Options importantes :
- `-DCMAKE_BUILD_TYPE=Release` : Compile en mode Release (optimisé)
- `-DBUILD_SHARED_LIBS=OFF` : Compile une bibliothèque statique (.lib) au lieu d'une DLL
- `-DENABLE_OPENSSL=OFF` : Désactiver OpenSSL si non disponible (optionnel)
- `-DENABLE_BZIP2=OFF` : Désactiver BZip2 si non disponible (optionnel)

### Étape 3 : Compiler

```cmd
cmake --build . --config Release
```

Ou avec Visual Studio :
```cmd
cmake --build . --config Release --target zip
```

### Étape 4 : Copier les fichiers

Le fichier `zip.lib` sera généré dans un sous-répertoire. Le trouver et le copier :

```cmd
copy Release\zip.lib ..\lib\zip.lib
```

Ou si vous utilisez une configuration différente :
```cmd
copy Debug\zip.lib ..\lib\zip.lib
```

## Vérification

Après la compilation, vérifier que les fichiers suivants existent :
- `Libs\libzip\lib\zip.lib` (bibliothèque compilée)
- `Libs\libzip\lib\zip.h` (header - déjà présent)
- `Libs\libzip\lib\zipconf.h` (configuration - créé manuellement)

## Résolution de problèmes

### Erreur : CMake non trouvé
- Installer CMake et s'assurer qu'il est dans le PATH
- Ou utiliser le chemin complet : `"C:\Program Files\CMake\bin\cmake.exe"`

### Erreur : Visual Studio non trouvé
- Installer Visual Studio avec les outils C++
- Ou spécifier le générateur : `cmake .. -G "Visual Studio 17 2022" -A x64`

### Erreur : Dépendances manquantes
- Les dépendances optionnelles (OpenSSL, BZip2, etc.) peuvent être désactivées
- Utiliser : `cmake .. -DENABLE_OPENSSL=OFF -DENABLE_BZIP2=OFF`

### Erreur de compilation
- Vérifier que vous utilisez la même architecture (x86 ou x64) que votre projet
- Spécifier l'architecture : `cmake .. -A x64` ou `cmake .. -A Win32`

## Architecture (x86 vs x64)

Pour compiler pour x64 :
```cmd
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release
```

Pour compiler pour x86 (Win32) :
```cmd
cmake .. -A Win32 -DCMAKE_BUILD_TYPE=Release
```

Assurez-vous que l'architecture correspond à celle de votre projet BackupJZ.

