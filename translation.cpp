#include "translation.h"
#include "preferences.h"
#include <map>
#include <string>

static Language currentLanguage = Language::French;

Language GetCurrentLanguage()
{
    Preferences prefs;
    if (LoadPreferences(prefs))
    {
        if (prefs.language == "en" || prefs.language == "english")
        {
            currentLanguage = Language::English;
            return Language::English;
        }
        else
        {
            currentLanguage = Language::French;
            return Language::French;
        }
    }
    return currentLanguage;
}

void SetCurrentLanguage(Language lang)
{
    currentLanguage = lang;
    Preferences prefs;
    LoadPreferences(prefs);
    prefs.language = (lang == Language::English) ? "en" : "fr";
    SavePreferences(prefs);
}

static std::map<std::string, std::map<Language, std::string>> translations = {
    {"FILE_MENU", {{Language::French, "Fichier"}, {Language::English, "File"}}},
    {"Backup_PREFERENCES", {{Language::French, "Préférences de Sauvegarde"}, {Language::English, "Backup Preferences"}}},
    {"APPLICATION_SETTINGS", {{Language::French, "Paramètres de l'application"}, {Language::English, "Application Settings"}}},
    {"QUIT", {{Language::French, "Quitter"}, {Language::English, "Quit"}}},
    {"RESTORE_MENU", {{Language::French, "Restaurer"}, {Language::English, "Restore"}}},
    {"RESTORE_Backup", {{Language::French, "Restaurer une sauvegarde"}, {Language::English, "Restore a Backup"}}},
    {"HELP_MENU", {{Language::French, "Aide"}, {Language::English, "Help"}}},
    {"ABOUT", {{Language::French, "À propos"}, {Language::English, "About"}}},
    
    {"SELECT_FOLDERS_TO_Backup", {{Language::French, "Sélectionnez les dossiers à sauvegarder"}, {Language::English, "Select folders to Backup"}}},
    {"ADD_FOLDER", {{Language::French, "Ajouter un dossier"}, {Language::English, "Add a folder"}}},
    {"REMOVE_SELECTION", {{Language::French, "Supprimer la sélection"}, {Language::English, "Remove selection"}}},
    {"SELECTED_FOLDERS", {{Language::French, "Dossiers sélectionnés"}, {Language::English, "Selected folders"}}},
    {"NO_FOLDER_SELECTED", {{Language::French, "Aucun dossier sélectionné"}, {Language::English, "No folder selected"}}},
    {"DESTINATION_FOLDER", {{Language::French, "Dossier de destination"}, {Language::English, "Destination folder"}}},
    {"CHOOSE_DESTINATION", {{Language::French, "Choisir le dossier de destination"}, {Language::English, "Choose destination folder"}}},
    {"NO_DESTINATION_SELECTED", {{Language::French, "(Aucun dossier sélectionné)"}, {Language::English, "(No folder selected)"}}},
    {"START_Backup", {{Language::French, "Démarrer la sauvegarde"}, {Language::English, "Start Backup"}}},
    {"GLOBAL_PROGRESS", {{Language::French, "Progression globale"}, {Language::English, "Global progress"}}},
    {"CURRENT_FILE", {{Language::French, "Fichier en cours"}, {Language::English, "Current file"}}},
    
    {"Backup_SETTINGS", {{Language::French, "Paramètres de sauvegarde"}, {Language::English, "Backup settings"}}},
    {"AUTO_Backup", {{Language::French, "Sauvegarde automatique"}, {Language::English, "Automatic Backup"}}},
    {"INTERVAL_HOURS", {{Language::French, "Intervalle (heures)"}, {Language::English, "Interval (hours)"}}},
    {"AUTO_COMPRESS", {{Language::French, "Compression automatique"}, {Language::English, "Automatic compression"}}},
    {"SECURITY", {{Language::French, "Sécurité"}, {Language::English, "Security"}}},
    {"ENCRYPT_BackupS", {{Language::French, "Chiffrer les sauvegardes"}, {Language::English, "Encrypt Backups"}}},
    {"ENCRYPTION_KEY", {{Language::French, "Clé de chiffrement"}, {Language::English, "Encryption key"}}},
    {"KEY_REQUIRED", {{Language::French, "⚠ Clé requise pour le chiffrement"}, {Language::English, "⚠ Key required for encryption"}}},
    {"KEY_CONFIGURED", {{Language::French, "✓ Clé configurée"}, {Language::English, "✓ Key configured"}}},
    {"MAX_BackupS", {{Language::French, "Nombre maximum de sauvegardes"}, {Language::English, "Maximum number of Backups"}}},
    {"AUTO_DELETE_OLD", {{Language::French, "Supprimer automatiquement les anciennes sauvegardes"}, {Language::English, "Automatically delete old Backups"}}},
    {"WHEN_MAX_REACHED", {{Language::French, "(Lorsque le nombre maximum est atteint)"}, {Language::English, "(When maximum is reached)"}}},
    {"INTERFACE", {{Language::French, "Interface"}, {Language::English, "Interface"}}},
    {"SHOW_NOTIFICATIONS", {{Language::French, "Afficher les notifications"}, {Language::English, "Show notifications"}}},
    {"MAINTENANCE", {{Language::French, "Maintenance"}, {Language::English, "Maintenance"}}},
    {"DELETE_OLD_BackupS", {{Language::French, "Vider les sauvegardes anciennes"}, {Language::English, "Delete old Backups"}}},
    {"SAVE", {{Language::French, "Enregistrer"}, {Language::English, "Save"}}},
    {"CANCEL", {{Language::French, "Annuler"}, {Language::English, "Cancel"}}},
    {"CLOSE", {{Language::French, "Fermer"}, {Language::English, "Close"}}},
    
    {"APPLICATION_SETTINGS_TITLE", {{Language::French, "Paramètres de l'application"}, {Language::English, "Application Settings"}}},
    {"APP_SETTINGS_DESCRIPTION", {{Language::French, "Ici vous pouvez configurer les paramètres généraux de l'application."}, {Language::English, "Here you can configure the general application settings."}}},
    {"APP_SETTINGS_Backup_NOTE", {{Language::French, "Les paramètres de sauvegarde sont disponibles dans le menu Préférences de Sauvegarde."}, {Language::English, "Backup settings are available in the Backup Preferences menu."}}},
    {"LANGUAGE", {{Language::French, "Langue"}, {Language::English, "Language"}}},
    {"LANGUAGE_FRENCH", {{Language::French, "Français"}, {Language::English, "French"}}},
    {"LANGUAGE_ENGLISH", {{Language::French, "Anglais"}, {Language::English, "English"}}},
    
    {"RESTORE_Backup_TITLE", {{Language::French, "Restaurer une sauvegarde"}, {Language::English, "Restore a Backup"}}},
    {"REFRESH", {{Language::French, "Actualiser"}, {Language::English, "Refresh"}}},
    {"AVAILABLE_BackupS", {{Language::French, "Sauvegardes disponibles"}, {Language::English, "Available Backups"}}},
    {"NO_Backup_FOUND", {{Language::French, "Aucune sauvegarde trouvée."}, {Language::English, "No Backup found."}}},
    {"SELECTED_Backup", {{Language::French, "Sauvegarde sélectionnée"}, {Language::English, "Selected Backup"}}},
    {"ENCRYPTED_Backup_DETECTED", {{Language::French, "⚠ Sauvegarde chiffrée détectée"}, {Language::English, "⚠ Encrypted Backup detected"}}},
    {"ENCRYPTION_KEY_WARNING", {{Language::French, "La clé de chiffrement dans les préférences doit être la même que celle utilisée lors de la sauvegarde pour pouvoir restaurer."}, {Language::English, "The encryption key in preferences must be the same as the one used during Backup to restore."}}},
    {"RESTORE_FOLDER", {{Language::French, "Dossier de restauration"}, {Language::English, "Restore folder"}}},
    {"BROWSE", {{Language::French, "Parcourir..."}, {Language::English, "Browse..."}}},
    {"RESTORING", {{Language::French, "Restauration en cours..."}, {Language::English, "Restoring..."}}},
    {"RESTORE", {{Language::French, "Restaurer"}, {Language::English, "Restore"}}},
    
    {"Backup_IN_PROGRESS", {{Language::French, "Sauvegarde en cours..."}, {Language::English, "Backup in progress..."}}},
    {"Backup_SUCCESS", {{Language::French, "Sauvegarde terminée avec succès!"}, {Language::English, "Backup completed successfully!"}}},
    {"Backup_ERROR", {{Language::French, "Erreur lors de la sauvegarde!"}, {Language::English, "Error during Backup!"}}},
    {"Backup_CANCELLED", {{Language::French, "Sauvegarde annulée!"}, {Language::English, "Backup cancelled!"}}},
    {"RESTORE_SUCCESS", {{Language::French, "Restauration terminée avec succès!"}, {Language::English, "Restore completed successfully!"}}},
    {"RESTORE_ERROR", {{Language::French, "Erreur lors de la restauration!"}, {Language::English, "Error during restore!"}}},
    {"OPERATION_SUCCESS", {{Language::French, "Opération réussie"}, {Language::English, "Operation successful"}}},
    {"OK", {{Language::French, "OK"}, {Language::English, "OK"}}},
    {"INITIALIZING", {{Language::French, "Initialisation..."}, {Language::English, "Initializing..."}}},
    {"GLOBAL_PROGRESS_LABEL", {{Language::French, "Progression globale:"}, {Language::English, "Global progress:"}}},
    {"CURRENT_FILE_LABEL", {{Language::French, "Fichier en cours:"}, {Language::English, "Current file:"}}},
    {"RESTORING_IN_PROGRESS", {{Language::French, "Restauration en cours..."}, {Language::English, "Restoring in progress..."}}},
    {"STARTING_RESTORE", {{Language::French, "Démarrage de la restauration..."}, {Language::English, "Starting restore..."}}},
    {"PREPARING_RESTORE", {{Language::French, "Préparation de la restauration..."}, {Language::English, "Preparing restore..."}}},
    {"ERROR_NO_DESTINATION_FOLDER", {{Language::French, "Erreur: Aucun dossier de destination sélectionné!"}, {Language::English, "Error: No destination folder selected!"}}},
    {"ERROR_NO_RESTORE_FOLDER", {{Language::French, "Erreur: Veuillez sélectionner un dossier de restauration!"}, {Language::English, "Error: Please select a restore folder!"}}},
    {"PATH_LABEL", {{Language::French, "Chemin:"}, {Language::English, "Path:"}}},
    {"COMPRESSING_FOLDER", {{Language::French, "Compression de: "}, {Language::English, "Compressing: "}}},
    {"ENCRYPTING_FOLDER", {{Language::French, "Chiffrement de: "}, {Language::English, "Encrypting: "}}},
    {"DECRYPTING_FILE", {{Language::French, "Déchiffrement du fichier..."}, {Language::English, "Decrypting file..."}}},
    {"EXTRACTING_FILES", {{Language::French, "Extraction des fichiers..."}, {Language::English, "Extracting files..."}}},
    {"AUTO_Backup_IN_PROGRESS", {{Language::French, "Sauvegarde automatique en cours..."}, {Language::English, "Automatic Backup in progress..."}}},
    {"AUTO_Backup_SUCCESS", {{Language::French, "Sauvegarde automatique terminée avec succès!"}, {Language::English, "Automatic Backup completed successfully!"}}},
    {"AUTO_Backup_ERROR", {{Language::French, "Erreur lors de la sauvegarde automatique!"}, {Language::English, "Error during automatic Backup!"}}},
    {"FILES", {{Language::French, "fichiers"}, {Language::English, "files"}}},
    
    {"ABOUT_TITLE", {{Language::French, "BackupJZ par CSystem"}, {Language::English, "BackupJZ by CSystem"}}},
    {"ABOUT_DESCRIPTION", {{Language::French, "Application de sauvegarde"}, {Language::English, "Backup application"}}},
    {"ABOUT_DEVELOPED", {{Language::French, "Développé avec ImGui"}, {Language::English, "Developed with ImGui"}}},
};

const char* Translate(const char* key)
{
    Language lang = GetCurrentLanguage();
    auto it = translations.find(key);
    if (it != translations.end())
    {
        auto langIt = it->second.find(lang);
        if (langIt != it->second.end())
        {
            return langIt->second.c_str();
        }
    }
    return key;
}

