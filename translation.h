#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <string>

enum class Language {
    French = 0,
    English = 1
};

Language GetCurrentLanguage();
void SetCurrentLanguage(Language lang);

const char* Translate(const char* key);

namespace TR {
    constexpr const char* FILE_MENU = "FILE_MENU";
    constexpr const char* Backup_PREFERENCES = "Backup_PREFERENCES";
    constexpr const char* APPLICATION_SETTINGS = "APPLICATION_SETTINGS";
    constexpr const char* QUIT = "QUIT";
    constexpr const char* RESTORE_MENU = "RESTORE_MENU";
    constexpr const char* RESTORE_Backup = "RESTORE_Backup";
    constexpr const char* HELP_MENU = "HELP_MENU";
    constexpr const char* ABOUT = "ABOUT";
    
    constexpr const char* SELECT_FOLDERS_TO_Backup = "SELECT_FOLDERS_TO_Backup";
    constexpr const char* ADD_FOLDER = "ADD_FOLDER";
    constexpr const char* REMOVE_SELECTION = "REMOVE_SELECTION";
    constexpr const char* SELECTED_FOLDERS = "SELECTED_FOLDERS";
    constexpr const char* NO_FOLDER_SELECTED = "NO_FOLDER_SELECTED";
    constexpr const char* DESTINATION_FOLDER = "DESTINATION_FOLDER";
    constexpr const char* CHOOSE_DESTINATION = "CHOOSE_DESTINATION";
    constexpr const char* NO_DESTINATION_SELECTED = "NO_DESTINATION_SELECTED";
    constexpr const char* START_Backup = "START_Backup";
    constexpr const char* GLOBAL_PROGRESS = "GLOBAL_PROGRESS";
    constexpr const char* CURRENT_FILE = "CURRENT_FILE";
    
    constexpr const char* Backup_SETTINGS = "Backup_SETTINGS";
    constexpr const char* AUTO_Backup = "AUTO_Backup";
    constexpr const char* INTERVAL_HOURS = "INTERVAL_HOURS";
    constexpr const char* AUTO_COMPRESS = "AUTO_COMPRESS";
    constexpr const char* SECURITY = "SECURITY";
    constexpr const char* ENCRYPT_BackupS = "ENCRYPT_BackupS";
    constexpr const char* ENCRYPTION_KEY = "ENCRYPTION_KEY";
    constexpr const char* KEY_REQUIRED = "KEY_REQUIRED";
    constexpr const char* KEY_CONFIGURED = "KEY_CONFIGURED";
    constexpr const char* MAX_BackupS = "MAX_BackupS";
    constexpr const char* AUTO_DELETE_OLD = "AUTO_DELETE_OLD";
    constexpr const char* WHEN_MAX_REACHED = "WHEN_MAX_REACHED";
    constexpr const char* INTERFACE = "INTERFACE";
    constexpr const char* SHOW_NOTIFICATIONS = "SHOW_NOTIFICATIONS";
    constexpr const char* MAINTENANCE = "MAINTENANCE";
    constexpr const char* DELETE_OLD_BackupS = "DELETE_OLD_BackupS";
    constexpr const char* SAVE = "SAVE";
    constexpr const char* CANCEL = "CANCEL";
    constexpr const char* CLOSE = "CLOSE";
    
    constexpr const char* APPLICATION_SETTINGS_TITLE = "APPLICATION_SETTINGS_TITLE";
    constexpr const char* APP_SETTINGS_DESCRIPTION = "APP_SETTINGS_DESCRIPTION";
    constexpr const char* APP_SETTINGS_Backup_NOTE = "APP_SETTINGS_Backup_NOTE";
    constexpr const char* LANGUAGE = "LANGUAGE";
    constexpr const char* LANGUAGE_FRENCH = "LANGUAGE_FRENCH";
    constexpr const char* LANGUAGE_ENGLISH = "LANGUAGE_ENGLISH";
    
    constexpr const char* RESTORE_Backup_TITLE = "RESTORE_Backup_TITLE";
    constexpr const char* REFRESH = "REFRESH";
    constexpr const char* AVAILABLE_BackupS = "AVAILABLE_BackupS";
    constexpr const char* NO_Backup_FOUND = "NO_Backup_FOUND";
    constexpr const char* SELECTED_Backup = "SELECTED_Backup";
    constexpr const char* ENCRYPTED_Backup_DETECTED = "ENCRYPTED_Backup_DETECTED";
    constexpr const char* ENCRYPTION_KEY_WARNING = "ENCRYPTION_KEY_WARNING";
    constexpr const char* RESTORE_FOLDER = "RESTORE_FOLDER";
    constexpr const char* BROWSE = "BROWSE";
    constexpr const char* RESTORING = "RESTORING";
    constexpr const char* RESTORE = "RESTORE";
    
    constexpr const char* Backup_IN_PROGRESS = "Backup_IN_PROGRESS";
    constexpr const char* Backup_SUCCESS = "Backup_SUCCESS";
    constexpr const char* Backup_ERROR = "Backup_ERROR";
    constexpr const char* Backup_CANCELLED = "Backup_CANCELLED";
    constexpr const char* RESTORE_SUCCESS = "RESTORE_SUCCESS";
    constexpr const char* RESTORE_ERROR = "RESTORE_ERROR";
    constexpr const char* OPERATION_SUCCESS = "OPERATION_SUCCESS";
    constexpr const char* OK = "OK";
    constexpr const char* INITIALIZING = "INITIALIZING";
    constexpr const char* GLOBAL_PROGRESS_LABEL = "GLOBAL_PROGRESS_LABEL";
    constexpr const char* CURRENT_FILE_LABEL = "CURRENT_FILE_LABEL";
    constexpr const char* RESTORING_IN_PROGRESS = "RESTORING_IN_PROGRESS";
    constexpr const char* STARTING_RESTORE = "STARTING_RESTORE";
    constexpr const char* PREPARING_RESTORE = "PREPARING_RESTORE";
    constexpr const char* ERROR_NO_DESTINATION_FOLDER = "ERROR_NO_DESTINATION_FOLDER";
    constexpr const char* ERROR_NO_RESTORE_FOLDER = "ERROR_NO_RESTORE_FOLDER";
    constexpr const char* PATH_LABEL = "PATH_LABEL";
    constexpr const char* COMPRESSING_FOLDER = "COMPRESSING_FOLDER";
    constexpr const char* ENCRYPTING_FOLDER = "ENCRYPTING_FOLDER";
    constexpr const char* DECRYPTING_FILE = "DECRYPTING_FILE";
    constexpr const char* EXTRACTING_FILES = "EXTRACTING_FILES";
    constexpr const char* AUTO_Backup_IN_PROGRESS = "AUTO_Backup_IN_PROGRESS";
    constexpr const char* AUTO_Backup_SUCCESS = "AUTO_Backup_SUCCESS";
    constexpr const char* AUTO_Backup_ERROR = "AUTO_Backup_ERROR";
    constexpr const char* FILES = "FILES";
    
    constexpr const char* ABOUT_TITLE = "ABOUT_TITLE";
    constexpr const char* ABOUT_DESCRIPTION = "ABOUT_DESCRIPTION";
    constexpr const char* ABOUT_DEVELOPED = "ABOUT_DEVELOPED";
}

#endif

