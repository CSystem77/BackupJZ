# Installation de CMake pour Windows

## Méthode 1 : Installation via winget (Recommandé - Rapide)

Ouvrir PowerShell en tant qu'administrateur et exécuter :

```powershell
winget install Kitware.CMake
```

Après l'installation, **fermer et rouvrir** le terminal pour que le PATH soit mis à jour.

## Méthode 2 : Installation manuelle

1. **Télécharger CMake** :
   - Aller sur : https://cmake.org/download/
   - Télécharger "Windows x64 Installer" (fichier .msi)

2. **Installer CMake** :
   - Exécuter le fichier téléchargé
   - **IMPORTANT** : Cocher "Add CMake to system PATH" pendant l'installation
   - Suivre les étapes d'installation

3. **Vérifier l'installation** :
   - Fermer et rouvrir le terminal
   - Exécuter : `cmake --version`

## Méthode 3 : Installation portable (Sans installation)

1. **Télécharger CMake portable** :
   - Aller sur : https://cmake.org/download/
   - Télécharger "Windows x64 ZIP" (fichier .zip)

2. **Extraire** :
   - Extraire dans un dossier, par exemple : `C:\Tools\cmake`

3. **Utiliser le chemin complet** :
   - Modifier le script `build_libzip.bat` pour utiliser le chemin complet
   - Ou ajouter manuellement au PATH temporairement

## Vérification

Après l'installation, vérifier que CMake fonctionne :

```cmd
cmake --version
```

Vous devriez voir quelque chose comme :
```
cmake version 3.28.0
CMake suite maintained and supported by Kitware
```

## Si CMake est installé mais pas dans le PATH

Si CMake est installé mais pas trouvé, vous pouvez :

1. **Ajouter manuellement au PATH** :
   - Chercher "Variables d'environnement" dans Windows
   - Ajouter `C:\Program Files\CMake\bin` au PATH système
   - Redémarrer le terminal

2. **Utiliser le chemin complet dans le script** :
   - Modifier `build_libzip.bat` pour utiliser le chemin complet vers cmake.exe

## Alternative : Utiliser Visual Studio avec CMake intégré

Si vous avez Visual Studio 2019 ou 2022, CMake peut être inclus :
- Ouvrir Visual Studio
- Aller dans "Extensions" > "Gérer les extensions"
- Chercher "CMake Tools for Visual Studio"

