#ifndef Backup_H
#define Backup_H

#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <windows.h>
#include "preferences.h"

bool CompressFolder(const std::wstring& sourcePath, const std::wstring& zipPath, HANDLE* processHandle = nullptr, std::mutex* processMutex = nullptr);
void SaveBackupList(const std::vector<std::wstring>& folders, const std::wstring& destinationPath);
bool PerformBackup(const std::vector<std::wstring>& folders, const std::wstring& destinationPath, const Preferences& prefs, std::atomic<float>* progress = nullptr, std::atomic<float>* currentFileProgress = nullptr, std::mutex* statusMutex = nullptr, std::string* currentStatus = nullptr, std::string* currentFileName = nullptr, std::atomic<bool>* shouldCancel = nullptr);
bool RestoreBackup(const std::wstring& BackupPath, const std::wstring& restorePath, std::atomic<float>* progress = nullptr, std::string* statusMessage = nullptr);
void CheckAndPerformAutoBackup(const std::vector<std::wstring>& folders, const std::wstring& destinationPath, Preferences& prefs, bool* isBackingUp = nullptr, std::atomic<float>* progress = nullptr, std::atomic<float>* currentFileProgress = nullptr, std::mutex* statusMutex = nullptr, std::string* currentStatus = nullptr, std::string* currentFileName = nullptr, std::string* statusMessage = nullptr, HWND hwnd = nullptr);
void DeleteOldBackups(const std::wstring& destinationPath, int maxBackups);

#endif
