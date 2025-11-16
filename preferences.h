#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <string>
#include <ctime>

struct Preferences
{
    bool autoBackup = false;
    int BackupInterval = 1;
    bool compressOnBackup = true;
    bool showNotifications = true;
    int maxBackups = 10;
    bool autoDeleteOldBackups = false;
    bool encryptBackups = false;
    std::string encryptionKey = "";
    std::time_t lastBackupTime = 0;
    std::string language = "fr";
};

void SavePreferences(const Preferences& prefs);
bool LoadPreferences(Preferences& prefs);
std::wstring GetPreferencesFilePath();

#endif
