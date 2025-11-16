# BackupJZ

![BackupJZ Demo](Demo/BackupJZ.png)

**BackupJZ** est une application de sauvegarde développée en C++ avec une interface graphique intuitive. Elle permet de sauvegarder, compresser, chiffrer et restaurer vos fichiers et dossiers de manière sécurisée et automatisée. 

Attention : Le système de chiffrement peut être détecté comme un virus (Chiffrement en XOR).

## Table des matières

- [Fonctionnalités](#-fonctionnalités)
- [Bibliothèques utilisées](#-bibliothèques-utilisées)
- [Installation](#-installation)
- [Compilation](#-compilation)
- [Utilisation](#-utilisation)
- [Structure du projet](#-structure-du-projet)
- [Auteur](#-auteur)

## Fonctionnalités

### Sauvegarde
- **Sauvegarde multiple** : Sélectionnez plusieurs dossiers à sauvegarder simultanément
- **Compression automatique** : Compression des sauvegardes au format ZIP pour économiser l'espace disque
- **Sauvegarde automatique** : Planification de sauvegardes automatiques à intervalles réguliers (configurable en heures)
- **Gestion des versions** : Conservation d'un nombre configurable de sauvegardes avec suppression automatique des anciennes versions

### Sécurité
- **Chiffrement optionnel** : Chiffrement des sauvegardes avec une clé personnalisée.
- **Format sécurisé** : Les sauvegardes chiffrées utilisent l'extension `.jz` pour une identification facile
- **Protection des données** : Chiffrement XOR combiné avec encodage Base64 pour une sécurité renforcée

### Restauration
- **Restauration complète** : Restauration de sauvegardes complètes ou sélectives
- **Détection automatique** : Détection automatique des sauvegardes chiffrées avec demande de clé
- **Interface intuitive** : Liste des sauvegardes disponibles avec sélection facile

### Interface utilisateur
- **Interface moderne** : Interface graphique développée avec ImGui offrant une expérience utilisateur fluide
- **Multilingue** : Support du français et de l'anglais avec changement de langue en temps réel
- **Barre de progression** : Suivi en temps réel de la progression des sauvegardes et restaurations
- **Notifications système** : Notifications Windows pour les opérations importantes
- **Minimisation dans la barre système** : L'application peut fonctionner en arrière-plan avec icône dans la zone de notification

### Préférences et configuration
- **Préférences de sauvegarde** :
  - Activation/désactivation de la sauvegarde automatique
  - Intervalle de sauvegarde automatique (en heures)
  - Compression automatique
  - Chiffrement avec clé personnalisée
  - Nombre maximum de sauvegardes à conserver
  - Suppression automatique des anciennes sauvegardes
  - Affichage des notifications
  
- **Paramètres de l'application** :
  - Sélection de la langue (Français/Anglais)
  - Sauvegarde automatique de la configuration

### Suivi et monitoring
- **Progression détaillée** : Affichage de la progression globale et par fichier
- **Statut en temps réel** : Messages de statut détaillés pendant les opérations
- **Historique** : Suivi des dernières sauvegardes effectuées

## Bibliothèques utilisées

### Interface graphique
- **ImGui** : Bibliothèque d'interface utilisateur immédiate pour C++
  - Backend : ImGui Win32 + OpenGL3
  - Version : Dernière version stable
  - Utilisation : Interface graphique complète, widgets, menus, popups

### Compression
- **libzip** : Bibliothèque pour la création et la manipulation d'archives ZIP
  - Support des formats de compression : ZIP standard
  - Utilisation : Compression et décompression des sauvegardes

### Algorithmes de compression
- **zlib** : Compression sans perte (déflate)
- **bzip2 (bz2)** : Compression avec meilleur ratio
- **LZMA** : Compression avec ratio élevé
- **Zstandard (zstd)** : Compression rapide et efficace

### Graphiques et rendu
- **OpenGL** : API graphique pour le rendu de l'interface
  - Version : OpenGL 3.x
  - Utilisation : Rendu de l'interface ImGui

### Système Windows
- **Win32 API** : API Windows native
  - Fonctions : Gestion des fenêtres, boîtes de dialogue, notifications système
  - Shell32 : Sélection de dossiers
  - GDI+ : Gestion des icônes et images

### Standard C++
- **C++17** : Standard C++ moderne
  - Utilisation : Threads, atomics, mutex pour les opérations asynchrones
  - STL : Vecteurs, chaînes, algorithmes

## Installation

### Prérequis
- **Windows 10** ou supérieur
- **Visual Studio 2019** ou supérieur (avec outilset v143)
- **CMake** (pour compiler libzip si nécessaire)
- **vcpkg** (optionnel, pour gérer les dépendances)

### Dépendances incluses
Le projet inclut déjà les bibliothèques nécessaires dans le dossier `Libs/` :
- `Libs/imGui/` : Bibliothèque ImGui complète
- `Libs/libzip/` : Bibliothèque libzip
- `Libs/x64-windows/` : Bibliothèques compilées (lib, dll, headers)

## Compilation

### Avec Visual Studio

1. **Ouvrir le projet** :
   ```
   Ouvrir BackupJZ.sln dans Visual Studio
   ```

2. **Configurer la plateforme** :
   - Sélectionner `x64` comme plateforme cible
   - Choisir `Debug` ou `Release` selon vos besoins

3. **Compiler** :
   - Menu : `Générer` > `Générer la solution` (ou `Ctrl+Shift+B`)
   - L'exécutable sera généré dans `Debug/` ou `Release/`

### Configuration du projet

Le fichier `BackupJZ.vcxproj` contient toutes les configurations nécessaires :
- **Include directories** :
  - `.\Libs\imGui`
  - `.\Libs\imGui\backends`
  - `.\Libs\libzip\lib`

- **Library directories** :
  - `.\Libs\libzip\lib`
  - `.\Libs\x64-windows\lib`

- **Libraries liées** :
  - `opengl32.lib`
  - `zip.lib`
  - `zlib.lib`
  - `bz2.lib`
  - `lzma.lib`
  - `zstd.lib`

- **Standard C++** : C++17

## Utilisation

### Première utilisation

1. **Lancer l'application** : Exécutez `BackupJZ.exe`

2. **Sélectionner les dossiers** :
   - Cliquez sur "Ajouter un dossier"
   - Sélectionnez les dossiers à sauvegarder
   - Les dossiers sélectionnés apparaissent dans la liste

3. **Choisir la destination** :
   - Cliquez sur "Choisir le dossier de destination"
   - Sélectionnez où enregistrer les sauvegardes

4. **Configurer les préférences** (optionnel) :
   - Cliquez sur "Préférences de Sauvegarde"
   - Configurez les options selon vos besoins :
     - Sauvegarde automatique
     - Compression
     - Chiffrement
     - Nombre maximum de sauvegardes

5. **Démarrer la sauvegarde** :
   - Cliquez sur "Démarrer la sauvegarde"
   - Suivez la progression dans l'interface

### Restauration d'une sauvegarde

1. **Ouvrir le menu Restaurer** :
   - Menu : `Restaurer` > `Restaurer une sauvegarde`

2. **Sélectionner une sauvegarde** :
   - Cliquez sur "Actualiser" pour lister les sauvegardes disponibles
   - Sélectionnez la sauvegarde à restaurer

3. **Choisir le dossier de restauration** :
   - Entrez le chemin ou utilisez "Parcourir"
   - Pour les sauvegardes chiffrées, la clé sera demandée automatiquement

4. **Lancer la restauration** :
   - Cliquez sur "Restaurer"
   - Suivez la progression

### Sauvegarde automatique

1. **Activer la sauvegarde automatique** :
   - Menu : `Fichier` > `Préférences de Sauvegarde`
   - Cochez "Sauvegarde automatique"
   - Définissez l'intervalle en heures

2. **L'application vérifie automatiquement** :
   - L'application vérifie périodiquement si une sauvegarde est nécessaire
   - Les sauvegardes automatiques s'exécutent en arrière-plan
   - Des notifications sont affichées (si activées)

## Structure du projet

```
BackupJZ/
├── Demo/
│   └── BackupJZ.png          # Capture d'écran de démonstration
├── Libs/
│   ├── imGui/                # Bibliothèque ImGui
│   │   ├── backends/         # Backends ImGui (Win32, OpenGL3)
│   │   └── ...
│   ├── libzip/               # Bibliothèque libzip
│   │   ├── lib/              # Headers et sources libzip
│   │   └── ...
│   └── x64-windows/          # Bibliothèques compilées
│       ├── bin/              # DLLs
│       ├── include/          # Headers
│       └── lib/              # Bibliothèques statiques
├── main.cpp                  # Point d'entrée principal
├── window.cpp/h              # Gestion des fenêtres Windows
├── backup.cpp/h              # Logique de sauvegarde
├── backup_tracking.cpp/h     # Suivi des sauvegardes
├── encryption.cpp/h          # Chiffrement/déchiffrement
├── config.cpp/h              # Gestion de la configuration
├── preferences.cpp/h         # Gestion des préférences
├── file_utils.cpp/h          # Utilitaires de fichiers
├── translation.cpp/h         # Système de traduction
├── logo_embedded.h           # Logo intégré
├── app_icon.ico              # Icône de l'application
├── app_icon.rc               # Ressource de l'icône
├── BackupJZ.sln              # Solution Visual Studio
├── BackupJZ.vcxproj          # Projet Visual Studio
└── README.md                 # Ce fichier
```

## Fonctionnalités avancées

### Chiffrement
- **Algorithme** : Chiffrement XOR avec clé personnalisée + encodage Base64
- **Format** : Les fichiers chiffrés utilisent l'extension `.jz`
- **Sécurité** : La clé est stockée de manière sécurisée dans les préférences

### Compression
- **Format** : ZIP standard compatible avec tous les décompresseurs
- **Algorithmes supportés** : zlib, bzip2, LZMA, Zstandard
- **Efficacité** : Compression optimale pour réduire la taille des sauvegardes

### Sauvegarde automatique
- **Vérification périodique** : L'application vérifie automatiquement si une sauvegarde est nécessaire
- **Exécution en arrière-plan** : Les sauvegardes automatiques ne bloquent pas l'interface
- **Notifications** : Notifications système pour les succès et erreurs

### Gestion des versions
- **Conservation configurable** : Nombre maximum de sauvegardes à conserver
- **Suppression automatique** : Suppression des anciennes sauvegardes quand la limite est atteinte
- **Nomenclature** : Les sauvegardes sont nommées avec date et heure

## Dépannage

### L'application ne démarre pas
- Vérifiez que toutes les DLL sont présentes dans le dossier de l'exécutable
- Vérifiez que les bibliothèques sont correctement liées

### Erreur de compression
- Vérifiez que le dossier de destination a suffisamment d'espace
- Vérifiez les permissions d'écriture sur le dossier de destination

### Erreur de restauration
- Vérifiez que la sauvegarde n'est pas corrompue
- Pour les sauvegardes chiffrées, vérifiez que la clé est correcte
- Vérifiez que le dossier de restauration existe et est accessible en écriture

## Auteur

**BackupJZ** par **CSystem**

Développé avec ImGui et les bibliothèques open-source mentionnées ci-dessus.

## Licence

Ce projet utilise plusieurs bibliothèques open-source :
- **ImGui** : Licence MIT
- **libzip** : Licence BSD
- **zlib** : Licence zlib
- **bzip2** : Licence BSD
- **LZMA** : Domaine public
- **Zstandard** : Licence BSD

---


