#include "preferences.h"
#include "file_utils.h"
#include <windows.h>
#include <fstream>
#include <sstream>

std::wstring GetPreferencesFilePath()
{
    wchar_t exePath[MAX_PATH];
    DWORD pathLen = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    
    if (pathLen > 0 && pathLen < MAX_PATH)
    {
        std::wstring exePathStr = exePath;
        size_t lastSlash = exePathStr.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos)
        {
            std::wstring prefsPath = exePathStr.substr(0, lastSlash + 1);
            prefsPath += L"pref.ini.jz";
            return prefsPath;
        }
    }
    
    return L"pref.ini.jz";
}

void SavePreferences(const Preferences& prefs)
{
    std::wstring prefsPath = GetPreferencesFilePath();
    std::string prefsPathStr = WStringToString(prefsPath);
    
    std::ostringstream oss;
    oss << "AUTO_Backup:" << (prefs.autoBackup ? "1" : "0") << "\n";
    oss << "Backup_INTERVAL:" << prefs.BackupInterval << "\n";
    oss << "COMPRESS_ON_Backup:" << (prefs.compressOnBackup ? "1" : "0") << "\n";
    oss << "SHOW_NOTIFICATIONS:" << (prefs.showNotifications ? "1" : "0") << "\n";
    oss << "MAX_BackupS:" << prefs.maxBackups << "\n";
    oss << "AUTO_DELETE_OLD_BackupS:" << (prefs.autoDeleteOldBackups ? "1" : "0") << "\n";
    oss << "ENCRYPT_BackupS:" << (prefs.encryptBackups ? "1" : "0") << "\n";
    oss << "ENCRYPTION_KEY:" << prefs.encryptionKey << "\n";
    oss << "LAST_Backup_TIME:" << prefs.lastBackupTime << "\n";
    oss << "LANGUAGE:" << prefs.language << "\n";
    
    std::string content = oss.str();
    const std::string key = "Backupjz";
    std::string encrypted = content;
    
    std::ofstream file(prefsPathStr, std::ios::out | std::ios::trunc);
    if (file.is_open())
    {
        file << encrypted;
        file.close();
    }
}

bool LoadPreferences(Preferences& prefs)
{

    prefs = Preferences();
    
    std::wstring prefsPath = GetPreferencesFilePath();
    std::string prefsPathStr = WStringToString(prefsPath);
    std::ifstream file(prefsPathStr, std::ios::in);
    
    if (!file.is_open())
    {
        return false;
    }
    
    std::string line;
    bool foundAutoDelete = false;
    
    while (std::getline(file, line))
    {

        if (!line.empty())
        {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
        }

        if (!line.empty())
        {
            line.erase(line.find_last_not_of(" \r\n\t") + 1);
        }
        
        if (line.empty())
        {
            continue;
        }
        
        if (line.find("AUTO_Backup:") == 0)
        {
            std::string value = line.substr(12);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            if (!value.empty())
            {
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
            }
            prefs.autoBackup = (value == "1");
        }
        else if (line.find("Backup_INTERVAL:") == 0)
        {
            std::string value = line.substr(16);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            if (!value.empty())
            {
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                try {
                    prefs.BackupInterval = std::stoi(value);
                } catch (...) {
                    prefs.BackupInterval = 1;
                }
            }
        }
        else if (line.find("COMPRESS_ON_Backup:") == 0)
        {
            std::string value = line.substr(19);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            if (!value.empty())
            {
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
            }
            prefs.compressOnBackup = (value == "1");
        }
        else if (line.find("SHOW_NOTIFICATIONS:") == 0)
        {
            std::string value = line.substr(19);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            if (!value.empty())
            {
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
            }
            prefs.showNotifications = (value == "1");
        }
        else if (line.find("MAX_BackupS:") == 0)
        {
            std::string value = line.substr(12);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            if (!value.empty())
            {
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                try {
                    prefs.maxBackups = std::stoi(value);
                } catch (...) {
                    prefs.maxBackups = 10;
                }
            }
        }
        else if (line.find("AUTO_DELETE_OLD_BackupS:") == 0)
        {

            size_t colonPos = line.find(':');
            std::string value;
            if (colonPos != std::string::npos && colonPos + 1 < line.length())
            {
                value = line.substr(colonPos + 1);
            }

            if (!value.empty())
            {
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                if (!value.empty())
                {
                    value.erase(value.find_last_not_of(" \t\r\n") + 1);
                }
            }

            prefs.autoDeleteOldBackups = (value == "1" || value == "true" || value == "TRUE" || value == "True");
            foundAutoDelete = true;
        }
        else if (line.find("ENCRYPT_BackupS:") == 0)
        {
            std::string value = line.substr(16);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            if (!value.empty())
            {
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
            }
            prefs.encryptBackups = (value == "1");
        }
        else if (line.find("ENCRYPTION_KEY:") == 0)
        {
            prefs.encryptionKey = line.substr(15);
            prefs.encryptionKey.erase(0, prefs.encryptionKey.find_first_not_of(" \t\r\n"));
            if (!prefs.encryptionKey.empty())
            {
                prefs.encryptionKey.erase(prefs.encryptionKey.find_last_not_of(" \t\r\n") + 1);
            }
        }
        else if (line.find("LAST_Backup_TIME:") == 0)
        {
            std::string value = line.substr(17);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            if (!value.empty())
            {
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                try {
                    prefs.lastBackupTime = std::stoll(value);
                } catch (...) {
                    prefs.lastBackupTime = 0;
                }
            }
        }
        else if (line.find("LANGUAGE:") == 0)
        {
            prefs.language = line.substr(9);
            prefs.language.erase(0, prefs.language.find_first_not_of(" \t\r\n"));
            if (!prefs.language.empty())
            {
                prefs.language.erase(prefs.language.find_last_not_of(" \t\r\n") + 1);
            }
            if (prefs.language.empty())
            {
                prefs.language = "fr";
            }
        }
    }
    
    file.close();
    
    return true;
}
