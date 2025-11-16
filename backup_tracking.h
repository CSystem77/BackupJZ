#ifndef Backup_TRACKING_H
#define Backup_TRACKING_H

#include <vector>
#include <string>
#include "file_utils.h"

std::wstring GetDataDirectoryPath();
std::wstring GetBaseNameFromBackupFile(const std::wstring& BackupFilename);
std::wstring GetTrackingFilePath(const std::wstring& baseName);
void RegisterBackupFile(const std::wstring& BackupFilename, const std::wstring& destinationPath);
void RemoveBackupFromTracking(const std::wstring& BackupFilename, const std::wstring& destinationPath);
std::vector<std::wstring> GetBackupFilesFromTracking(const std::wstring& baseName, const std::wstring& destinationPath);
void DeleteAllOldBackups(const std::wstring& destinationPath, int maxBackups);
std::vector<std::wstring> GetAllBackups(const std::wstring& destinationPath);

#endif
