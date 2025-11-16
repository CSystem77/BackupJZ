#include "Backup.h"
#include "file_utils.h"
#include "Backup_tracking.h"
#include "encryption.h"
#include "preferences.h"
#include "window.h"
#include "translation.h"
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <comdef.h>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <cstring>
#define ZIP_STATIC

#include <zipconf.h>
#include <zip.h>
#include <filesystem>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

static int AddFileToZip(zip_t* zip, const std::string& filePath, const std::string& basePath)
{
    namespace fs = std::filesystem;
    
    try
    {
        fs::path path(filePath);
        if (!fs::exists(path))
        {
            return -1;
        }
        
        if (fs::is_directory(path))
        {
            std::string relativePath = filePath.substr(basePath.length());
            std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
            if (!relativePath.empty() && relativePath.back() != '/')
            {
                relativePath += '/';
            }
            
            zip_dir_add(zip, relativePath.c_str(), ZIP_FL_ENC_UTF_8);
            
            for (const auto& entry : fs::recursive_directory_iterator(path))
            {
                if (fs::is_regular_file(entry))
                {
                    std::string entryPath = entry.path().string();
                    std::string relativeEntryPath = entryPath.substr(basePath.length());
                    std::replace(relativeEntryPath.begin(), relativeEntryPath.end(), '\\', '/');
                    
                    zip_source_t* source = zip_source_file(zip, entryPath.c_str(), 0, -1);
                    if (source)
                    {
                        zip_file_add(zip, relativeEntryPath.c_str(), source, ZIP_FL_ENC_UTF_8);
                    }
                }
            }
        }
        else if (fs::is_regular_file(path))
        {
            std::string relativePath = filePath.substr(basePath.length());
            std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
            
            zip_source_t* source = zip_source_file(zip, filePath.c_str(), 0, -1);
            if (source)
            {
                zip_file_add(zip, relativePath.c_str(), source, ZIP_FL_ENC_UTF_8);
            }
        }
    }
    catch (...)
    {
        return -1;
    }
    
    return 0;
}

bool CompressFolder(const std::wstring& sourcePath, const std::wstring& zipPath, HANDLE* , std::mutex*)
{
    std::string sourcePathStr = WStringToString(sourcePath);
    std::string zipPathStr = WStringToString(zipPath);
    
    if (std::filesystem::exists(zipPathStr))
    {
        std::filesystem::remove(zipPathStr);
    }
    
    std::filesystem::path zipPathFile(zipPathStr);
    if (zipPathFile.has_parent_path())
    {
        std::filesystem::create_directories(zipPathFile.parent_path());
    }
    
    int error = 0;
    zip_t* zip = zip_open(zipPathStr.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip)
    {
        return false;
    }
    
    std::filesystem::path sourcePathFile(sourcePathStr);
    std::string basePath = sourcePathFile.parent_path().string();
    if (!basePath.empty() && basePath.back() != '\\' && basePath.back() != '/')
    {
        basePath += "\\";
    }
    
    if (std::filesystem::is_directory(sourcePathStr))
    {
        // BIENTOT
    }
    bool success = (AddFileToZip(zip, sourcePathStr, basePath) == 0);
    
    if (zip_close(zip) != 0)
    {
        success = false;
    }
    
    return success;
}

void SaveBackupList(const std::vector<std::wstring>& folders, const std::wstring& destinationPath)
{
    std::wstring listPath = destinationPath + L"\\Backup_list_" + GetCurrentDateTimeString() + L".txt";
    std::string listPathStr = WStringToString(listPath);
    std::ofstream file(listPathStr);
    
    if (file.is_open())
    {
        file << "Liste des dossiers sauvegardes\n";
        file << "Date: " << GetFormattedDateTimeString() << "\n";
        file << "========================================\n\n";
        
        for (size_t i = 0; i < folders.size(); i++)
        {
            file << (i + 1) << ". " << WStringToString(folders[i]) << "\n";
        }
        
        file.close();
    }
}

void DeleteOldBackups(const std::wstring& destinationPath, int maxBackups)
{
    try
    {
        std::wstring destPath = destinationPath;
        if (destPath.empty() || maxBackups <= 0)
        {
            return;
        }
        
        if (destPath.back() != L'\\' && destPath.back() != L'/')
        {
            destPath += L"\\";
        }
        
        struct FileInfo
        {
            std::wstring path;
            std::wstring filename;
            std::wstring baseName;
            std::time_t dateFromFilename;
            FILETIME lastWriteTime;
        };
        
        std::wstring dataDir = GetDataDirectoryPath();
        WIN32_FIND_DATAW findData;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        
        std::wstring searchPattern = dataDir + L"\\*.csv";
        hFind = FindFirstFileW(searchPattern.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    std::wstring trackingFile = findData.cFileName;

                    size_t lastDot = trackingFile.find_last_of(L'.');
                    if (lastDot != std::wstring::npos)
                    {
                        std::wstring baseName = trackingFile.substr(0, lastDot);
                        
                        std::vector<std::wstring> BackupFiles = GetBackupFilesFromTracking(baseName, destinationPath);
                        
                        std::vector<FileInfo> fileInfos;
                        for (const auto& filePath : BackupFiles)
                        {

                            std::string filePathStr = WStringToString(filePath);
                            std::ifstream testFile(filePathStr);
                            if (!testFile.good())
                            {

                                size_t lastSlash = filePath.find_last_of(L"\\/");
                                std::wstring filename = (lastSlash != std::wstring::npos) ? 
                                    filePath.substr(lastSlash + 1) : filePath;
                                RemoveBackupFromTracking(filename, destinationPath);
                                continue;
                            }
                            
                            size_t lastSlash = filePath.find_last_of(L"\\/");
                            std::wstring filename = (lastSlash != std::wstring::npos) ? 
                                filePath.substr(lastSlash + 1) : filePath;
                            
                            FileInfo info;
                            info.filename = filename;
                            info.path = filePath;
                            info.baseName = baseName;
                            
                            WIN32_FIND_DATAW fileData;
                            HANDLE hFile = FindFirstFileW(filePath.c_str(), &fileData);
                            if (hFile != INVALID_HANDLE_VALUE)
                            {
                                info.lastWriteTime = fileData.ftLastWriteTime;
                                FindClose(hFile);
                            }
                            
                            info.dateFromFilename = ExtractDateFromFilename(filename);
                            fileInfos.push_back(info);
                        }
                        
                        std::sort(fileInfos.begin(), fileInfos.end(),
                            [](const FileInfo& a, const FileInfo& b) {
                                if (a.dateFromFilename != 0 && b.dateFromFilename != 0)
                                {
                                    return a.dateFromFilename < b.dateFromFilename;
                                }
                                if (a.dateFromFilename != 0) return false;
                                if (b.dateFromFilename != 0) return true;
                                return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) < 0;
                            });
                        
                        if (fileInfos.size() > static_cast<size_t>(maxBackups))
                        {
                            size_t filesToDelete = fileInfos.size() - maxBackups;
                            for (size_t i = 0; i < filesToDelete && i < fileInfos.size(); i++)
                            {
                                BOOL result = DeleteFileW(fileInfos[i].path.c_str());
                                if (result)
                                {

                                    RemoveBackupFromTracking(fileInfos[i].filename, destinationPath);
                                }
                            }
                        }
                    }
                }
            } while (FindNextFileW(hFind, &findData) != 0);
            FindClose(hFind);
        }
        
    }
    catch (...)
    {

    }
}

bool PerformBackup(const std::vector<std::wstring>& folders, const std::wstring& destinationPath, const Preferences& prefs, std::atomic<float>* progress, std::atomic<float>* currentFileProgress, std::mutex* statusMutex, std::string* currentStatus, std::string* currentFileName, std::atomic<bool>* shouldCancel)
{
    bool allSuccess = true;
    
    if (progress) progress->store(0.0f, std::memory_order_relaxed);
    if (currentFileProgress) currentFileProgress->store(0.0f, std::memory_order_relaxed);
    
    if (folders.empty())
    {
        if (progress) progress->store(100.0f, std::memory_order_relaxed);
        if (currentFileProgress) currentFileProgress->store(100.0f, std::memory_order_relaxed);
        return true;
    }
    
    size_t totalFiles = 0;
    unsigned long long totalSize = 0;
    
    if (statusMutex && currentStatus)
    {
        std::lock_guard<std::mutex> lock(*statusMutex);
        *currentStatus = "Analyse des dossiers...";
    }
    
    for (const auto& folder : folders)
    {
        FileCountInfo info = CountFilesRecursively(folder);
        totalFiles += info.totalFiles;
        totalSize += info.totalSize;
    }
    
    if (totalFiles == 0)
    {
        if (progress) progress->store(100.0f, std::memory_order_relaxed);
        if (currentFileProgress) currentFileProgress->store(100.0f, std::memory_order_relaxed);
        return true;
    }
    
    size_t processedFiles = 0;
    unsigned long long processedSize = 0;
    
    for (size_t i = 0; i < folders.size(); i++)
    {

        if (shouldCancel && shouldCancel->load(std::memory_order_acquire))
        {
            allSuccess = false;
            if (statusMutex && currentStatus)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentStatus = "Annulation...";
            }
            return false;
        }
        
        std::wstring sourcePath = folders[i];
        
        size_t lastSlash = sourcePath.find_last_of(L"\\/");
        std::wstring folderName = (lastSlash != std::wstring::npos) ? 
            sourcePath.substr(lastSlash + 1) : sourcePath;
        
        if (statusMutex && currentStatus)
        {
            std::lock_guard<std::mutex> lock(*statusMutex);
            *currentStatus = "Sauvegarde de: " + WStringToString(folderName) + " (" + std::to_string(i + 1) + "/" + std::to_string(folders.size()) + ")";
        }
        
        std::wstring BackupFileName;
        std::wstring BackupPath;
        bool isCompressed = false;
        
        FileCountInfo folderInfo = CountFilesRecursively(sourcePath);
        size_t folderFileCount = folderInfo.totalFiles;
        unsigned long long folderSizeBytes = folderInfo.totalSize;
        
        if (prefs.compressOnBackup)
        {

            if (statusMutex && currentStatus)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentStatus = std::string(Translate(TR::COMPRESSING_FOLDER)) + WStringToString(folderName) + " (" + std::to_string(folderFileCount) + " " + Translate(TR::FILES) + ")";
            }
            
            if (statusMutex && currentFileName)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentFileName = WStringToString(folderName) + ".zip";
            }
            
            BackupFileName = folderName + L"_" + GetCurrentDateTimeString() + L".zip";
            BackupPath = destinationPath + L"\\" + BackupFileName;
            
            if (currentFileProgress)
            {
                currentFileProgress->store(0.0f, std::memory_order_relaxed);
            }
            
            std::atomic<bool> compressionDone(false);
            std::atomic<bool> compressionCancelled(false);
            HANDLE compressProcessHandle = nullptr;
            std::mutex compressProcessMutex;
            
            std::thread compressThread([&]() {
                CompressFolder(sourcePath, BackupPath, &compressProcessHandle, &compressProcessMutex);
                compressionDone = true;
            });
            
            unsigned long long lastZipSize = 0;
            auto startTime = std::chrono::steady_clock::now();
            bool fileStarted = false;
            
            while (!compressionDone)
            {

                if (shouldCancel && shouldCancel->load(std::memory_order_acquire))
                {
                    allSuccess = false;
                    compressionCancelled = true;
                    if (statusMutex && currentStatus)
                    {
                        std::lock_guard<std::mutex> lock(*statusMutex);
                        *currentStatus = "Annulation...";
                    }

                    {
                        std::lock_guard<std::mutex> lock(compressProcessMutex);
                        if (compressProcessHandle)
                        {
                            TerminateProcess(compressProcessHandle, 1);
                            compressProcessHandle = nullptr;
                        }
                    }

                    compressThread.detach();
                    return false;
                }
                
                unsigned long long currentZipSize = GetFileSize(BackupPath);
                bool fileExists = FileExists(BackupPath);
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
                
                if (currentFileProgress && folderSizeBytes > 0)
                {

                    if (fileExists && currentZipSize > 0)
                    {
                        fileStarted = true;

                        unsigned long long estimatedFinalSize = (unsigned long long)(folderSizeBytes * 0.70);
                        if (estimatedFinalSize == 0) estimatedFinalSize = folderSizeBytes;
                        
                        float fileProgress = (std::min)(95.0f, (float)(currentZipSize * 100.0 / estimatedFinalSize));
                        currentFileProgress->store(fileProgress, std::memory_order_relaxed);
                        lastZipSize = currentZipSize;
                    }
                    else if (fileExists && currentZipSize == 0 && fileStarted)
                    {

                    }
                    else
                    {

                        const double estimatedSpeed = 10.0 * 1024 * 1024;
                        double estimatedTime = (double)folderSizeBytes / estimatedSpeed * 1000.0;
                        
                        if (estimatedTime > 0)
                        {

                            float timeProgress = (std::min)(95.0f, (float)(elapsed / estimatedTime * 95.0));

                            float currentValue = currentFileProgress->load(std::memory_order_relaxed);
                            if (timeProgress > currentValue || currentValue == 0.0f)
                            {
                                currentFileProgress->store(timeProgress, std::memory_order_relaxed);
                            }
                        }
                        else
                        {

                            float timeProgress = (std::min)(95.0f, (float)(elapsed / 1000.0));
                            currentFileProgress->store(timeProgress, std::memory_order_relaxed);
                        }
                    }
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            if (!(shouldCancel && shouldCancel->load(std::memory_order_acquire)))
            {
                compressThread.join();
                
                if (currentFileProgress)
                {
                    currentFileProgress->store(100.0f, std::memory_order_relaxed);
                }
                
                WIN32_FIND_DATAW findData;
                HANDLE hFile = FindFirstFileW(BackupPath.c_str(), &findData);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    allSuccess = false;
                    break;
                }
                FindClose(hFile);
            }
            else
            {

                break;
            }
            
            isCompressed = true;
            
            processedFiles += folderFileCount;
            processedSize += folderSizeBytes;
            if (progress && folders.size() > 0)
            {
                progress->store((static_cast<float>(i + 1) / static_cast<float>(folders.size())) * 100.0f, std::memory_order_relaxed);
            }
        }
        else
        {

            if (statusMutex && currentStatus)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentStatus = "Copie de: " + WStringToString(folderName) + " (" + std::to_string(folderFileCount) + " fichiers)";
            }
            
            if (statusMutex && currentFileName)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentFileName = WStringToString(folderName);
            }
            
            BackupFileName = folderName + L"_" + GetCurrentDateTimeString();
            BackupPath = destinationPath + L"\\" + BackupFileName;
            
            if (currentFileProgress)
            {
                currentFileProgress->store(0.0f, std::memory_order_relaxed);
            }
            
            std::atomic<bool> copyDone(false);
            bool copySuccess = false;
            
            std::thread copyThread([&sourcePath, &BackupPath, &copyDone, &copySuccess, shouldCancel]() {
                copySuccess = CopyDirectoryRecursive(sourcePath, BackupPath);
                copyDone = true;
            });
            
            while (!copyDone)
            {
                if (shouldCancel && shouldCancel->load(std::memory_order_acquire))
                {
                    allSuccess = false;
                    if (statusMutex && currentStatus)
                    {
                        std::lock_guard<std::mutex> lock(*statusMutex);
                        *currentStatus = "Annulation...";
                    }
                    copyThread.join();
                    return false;
                }
                
                FileCountInfo copiedInfo = CountFilesRecursively(BackupPath);
                unsigned long long currentCopiedSize = copiedInfo.totalSize;
                
                if (currentFileProgress && folderSizeBytes > 0)
                {
                    float copyProgress = (std::min)(95.0f, (float)(currentCopiedSize * 100.0 / folderSizeBytes));
                    currentFileProgress->store(copyProgress, std::memory_order_relaxed);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            if (!(shouldCancel && shouldCancel->load(std::memory_order_acquire)))
            {
                copyThread.join();
                
                if (currentFileProgress)
                {
                    currentFileProgress->store(100.0f, std::memory_order_relaxed);
                }
                
                if (!copySuccess)
                {
                    allSuccess = false;
                    break;
                }
            }
            else
            {
                break;
            }
            
            processedFiles += folderFileCount;
            processedSize += folderSizeBytes;
            if (progress && folders.size() > 0)
            {
                progress->store((static_cast<float>(i + 1) / static_cast<float>(folders.size())) * 100.0f, std::memory_order_relaxed);
            }
        }
        
        if (shouldCancel && shouldCancel->load(std::memory_order_acquire))
        {
            allSuccess = false;
            if (statusMutex && currentStatus)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentStatus = "Annulation...";
            }
            return false;
        }
        
        if (prefs.encryptBackups && !prefs.encryptionKey.empty() && isCompressed)
        {

            if (statusMutex && currentStatus)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentStatus = std::string(Translate(TR::ENCRYPTING_FOLDER)) + WStringToString(folderName);
            }
            
            if (statusMutex && currentFileName)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentFileName = WStringToString(folderName) + ".jz";
            }
            
            if (currentFileProgress)
            {
                currentFileProgress->store(0.0f, std::memory_order_relaxed);
            }
            
            EncryptZipFile(BackupPath, prefs.encryptionKey);

            size_t zipPos = BackupFileName.rfind(L".zip");
            if (zipPos != std::wstring::npos)
            {
                BackupFileName.replace(zipPos, 4, L".jz");
            }
            
            if (currentFileProgress)
            {
                currentFileProgress->store(100.0f, std::memory_order_relaxed);
            }
            
        }
        
        RegisterBackupFile(BackupFileName, destinationPath);
        
        if (prefs.maxBackups > 0)
        {
            std::wstring baseName = GetBaseNameFromBackupFile(BackupFileName);
            std::vector<std::wstring> BackupFiles = GetBackupFilesFromTracking(baseName, destinationPath);
            
            if (BackupFiles.size() > static_cast<size_t>(prefs.maxBackups))
            {

                struct FileInfo
                {
                    std::wstring path;
                    std::wstring filename;
                    std::time_t dateFromFilename;
                    FILETIME lastWriteTime;
                };
                
                std::vector<FileInfo> fileInfos;
                for (const auto& filePath : BackupFiles)
                {

                    WIN32_FIND_DATAW fileData;
                    HANDLE hFile = FindFirstFileW(filePath.c_str(), &fileData);
                    if (hFile == INVALID_HANDLE_VALUE)
                    {

                        size_t fileLastSlash = filePath.find_last_of(L"\\/");
                        std::wstring filename = (fileLastSlash != std::wstring::npos) ? 
                            filePath.substr(fileLastSlash + 1) : filePath;
                        RemoveBackupFromTracking(filename, destinationPath);
                        continue;
                    }
                    
                    FindClose(hFile);
                    
                    size_t lastSlash2 = filePath.find_last_of(L"\\/");
                    std::wstring filename = (lastSlash2 != std::wstring::npos) ? 
                        filePath.substr(lastSlash2 + 1) : filePath;
                    
                    FileInfo info;
                    info.filename = filename;
                    info.path = filePath;
                    info.lastWriteTime = fileData.ftLastWriteTime;
                    info.dateFromFilename = ExtractDateFromFilename(filename);
                    fileInfos.push_back(info);
                }
                
                std::sort(fileInfos.begin(), fileInfos.end(),
                    [](const FileInfo& a, const FileInfo& b) {
                        if (a.dateFromFilename != 0 && b.dateFromFilename != 0)
                        {
                            return a.dateFromFilename < b.dateFromFilename;
                        }
                        if (a.dateFromFilename != 0) return false;
                        if (b.dateFromFilename != 0) return true;
                        return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) < 0;
                    });
                
                size_t filesToDelete = fileInfos.size() - prefs.maxBackups;
                for (size_t j = 0; j < filesToDelete && j < fileInfos.size(); j++)
                {
                    BOOL result = DeleteFileW(fileInfos[j].path.c_str());
                    if (result)
                    {
                        RemoveBackupFromTracking(fileInfos[j].filename, destinationPath);
                    }
                    else
                    {

                        DWORD attrs = GetFileAttributesW(fileInfos[j].path.c_str());
                        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY))
                        {
                            if (DeleteDirectoryRecursive(fileInfos[j].path))
                            {
                                RemoveBackupFromTracking(fileInfos[j].filename, destinationPath);
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (progress)
    {
        progress->store(100.0f, std::memory_order_relaxed);
    }
    if (currentFileProgress)
    {
        currentFileProgress->store(100.0f, std::memory_order_relaxed);
    }
    
    if (statusMutex && currentStatus)
    {
        std::lock_guard<std::mutex> lock(*statusMutex);
        if (allSuccess)
        {
            *currentStatus = "Sauvegarde terminée avec succès!";
        }
        else
        {
            *currentStatus = "Erreur lors de la sauvegarde!";
        }
    }
    
    if (statusMutex && currentFileName)
    {
        std::lock_guard<std::mutex> lock(*statusMutex);
        *currentFileName = "";
    }
    
    if (allSuccess)
    {

    }
    
    return allSuccess;
}

bool RestoreBackup(const std::wstring& BackupPath, const std::wstring& restorePath, std::atomic<float>* progress, std::string* statusMessage)
{
    try
    {

        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(BackupPath.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        FindClose(hFind);
        
        DWORD attrs = GetFileAttributesW(BackupPath.c_str());
        bool isDirectory = (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
        
        if (isDirectory)
        {
            bool success = CopyDirectoryRecursive(BackupPath, restorePath);
            return success;
        }
        else
        {

            std::wstring BackupPathStr = BackupPath;
            
            bool isEncrypted = (BackupPathStr.length() > 3 && 
                               BackupPathStr.substr(BackupPathStr.length() - 3) == L".jz");
            
            std::wstring tempZipPath;
            if (isEncrypted)
            {

                tempZipPath = BackupPathStr.substr(0, BackupPathStr.length() - 3) + L"_temp.zip";
                
                Preferences prefs;
                LoadPreferences(prefs);
                
                if (prefs.encryptionKey.empty())
                {
                    return false;
                }
                
                std::atomic<float> decryptProgress(0.0f);
                if (statusMessage) *statusMessage = Translate(TR::DECRYPTING_FILE);
                if (progress) progress->store(0.0f, std::memory_order_relaxed);
                
                std::atomic<bool> decryptDone(false);
                std::thread decryptThread([&]() {
                    DecryptZipFile(BackupPathStr, tempZipPath, prefs.encryptionKey, &decryptProgress);
                    decryptDone = true;
                });
                
                float lastDecryptProgress = 0.0f;
                while (!decryptDone)
                {
                    if (progress)
                    {
                        float decryptValue = decryptProgress.load(std::memory_order_acquire);

                        float mappedProgress = decryptValue * 0.5f;

                        if (mappedProgress > lastDecryptProgress || lastDecryptProgress == 0.0f)
                        {
                            progress->store(mappedProgress, std::memory_order_relaxed);
                            lastDecryptProgress = mappedProgress;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                
                decryptThread.join();
                
                if (progress)
                {
                    float finalDecryptValue = decryptProgress.load(std::memory_order_acquire);
                    float finalMapped = finalDecryptValue * 0.5f;
                    progress->store((std::max)(50.0f, finalMapped), std::memory_order_relaxed);
                }
            }
            else
            {
                tempZipPath = BackupPathStr;
            }
            
            if (statusMessage) *statusMessage = Translate(TR::EXTRACTING_FILES);
            if (progress && !isEncrypted) progress->store(50.0f, std::memory_order_relaxed);
            
            std::string tempZipPathStr = WStringToString(tempZipPath);
            std::string restorePathStr = WStringToString(restorePath);
            
            std::filesystem::create_directories(restorePathStr);
            
            unsigned long long zipSize = GetFileSize(tempZipPath);
            std::atomic<bool> extractionDone(false);
            std::atomic<bool> extractionSuccess(false);
            std::thread extractionThread([&tempZipPathStr, &restorePathStr, &extractionDone, &extractionSuccess, &progress]() {
                try
                {
                    int error = 0;
                    zip_t* zip = zip_open(tempZipPathStr.c_str(), ZIP_RDONLY, &error);
                    if (!zip)
                    {
                        zip_error_t zipError;
                        zip_error_init_with_code(&zipError, error);
                        zip_error_fini(&zipError);
                        extractionSuccess = false;
                        extractionDone = true;
                        return;
                    }
                
                zip_int64_t numEntries = zip_get_num_entries(zip, 0);
                zip_int64_t processedEntries = 0;
                bool hasErrors = false;
                
                for (zip_int64_t i = 0; i < numEntries; i++)
                {
                    const char* name = zip_get_name(zip, i, ZIP_FL_ENC_UTF_8);
                    if (!name)
                    {
                        continue;
                    }
                    
                    std::string fullPath = restorePathStr + "\\" + name;
                    std::replace(fullPath.begin(), fullPath.end(), '/', '\\');
                    
                    std::filesystem::path filePath(fullPath);
                    if (filePath.has_parent_path())
                    {
                        std::filesystem::create_directories(filePath.parent_path());
                    }
                    
                    zip_stat_t stat;
                    if (zip_stat_index(zip, i, 0, &stat) == 0)
                    {
                        std::string entryName(name);
                        bool isDirectory = (!entryName.empty() && entryName.back() == '/');
                        
                        if (isDirectory)
                        {
                            std::filesystem::create_directories(filePath);
                        }
                        else if (stat.size > 0)
                        {
                            zip_file_t* file = zip_fopen_index(zip, i, 0);
                            if (file)
                            {
                                std::ofstream outFile(fullPath, std::ios::binary);
                                if (outFile.is_open())
                                {
                                    std::vector<char> buffer(8192);
                                    zip_uint64_t totalRead = 0;
                                    
                                    while (totalRead < stat.size)
                                    {
                                        zip_int64_t bytesRead = zip_fread(file, buffer.data(), buffer.size());
                                        if (bytesRead < 0)
                                        {
                                            hasErrors = true;
                                            break;
                                        }
                                        if (bytesRead == 0)
                                        {
                                            break;
                                        }
                                        outFile.write(buffer.data(), static_cast<size_t>(bytesRead));
                                        totalRead += static_cast<zip_uint64_t>(bytesRead);
                                    }
                                    outFile.close();
                                }
                                else
                                {
                                    hasErrors = true;
                                }
                                zip_fclose(file);
                            }
                            else
                            {
                                hasErrors = true;
                            }
                        }
                    }
                    
                    processedEntries++;
                    if (progress && numEntries > 0)
                    {
                        float extractProgress = 50.0f + (float)(processedEntries * 44.0 / numEntries);
                        progress->store((std::min)(94.0f, extractProgress), std::memory_order_relaxed);
                    }
                }
                
                    zip_close(zip);
                    extractionSuccess = !hasErrors;
                }
                catch (const std::exception&)
                {
                    extractionSuccess = false;
                }
                catch (...)
                {
                    extractionSuccess = false;
                }
                extractionDone = true;
            });
            
            unsigned long long lastExtractedSize = 0;
            unsigned long long lastFileCount = 0;
            auto startTime = std::chrono::steady_clock::now();
            float lastProgressValue = 50.0f;
            
            unsigned long long estimatedUncompressedSize = zipSize * 3;
            if (estimatedUncompressedSize == 0) estimatedUncompressedSize = zipSize * 10;
            
            if (progress)
            {
                progress->store(50.0f, std::memory_order_relaxed);
                
                while (!extractionDone)
                {
                    auto currentTime = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
                    
                    FileCountInfo extractedInfo = CountFilesRecursively(restorePath);
                    unsigned long long extractedSize = extractedInfo.totalSize;
                    unsigned long long extractedFiles = extractedInfo.totalFiles;
                    
                    float progressValue = lastProgressValue;
                    
                    if (extractedSize > lastExtractedSize || extractedFiles > lastFileCount)
                    {
                        if (estimatedUncompressedSize > 0 && extractedSize > 0)
                        {
                            float sizeProgress = (std::min)(94.0f, 50.0f + (float)(extractedSize * 44.0 / estimatedUncompressedSize));
                            progressValue = sizeProgress;
                        }
                        lastExtractedSize = extractedSize;
                        lastFileCount = extractedFiles;
                    }
                    else if (elapsed > 1000)
                    {
                        const double estimatedSpeed = 10.0 * 1024 * 1024;
                        double estimatedTime = (double)zipSize / estimatedSpeed * 1000.0;
                        
                        if (estimatedTime > 0 && elapsed < estimatedTime * 2)
                        {
                            float timeProgress = (std::min)(94.0f, 50.0f + (float)((elapsed - 1000) / 1000.0 * 0.5f));
                            progressValue = (std::max)(lastProgressValue, timeProgress);
                        }
                        else
                        {
                            float timeProgress = (std::min)(94.0f, lastProgressValue + 0.1f);
                            progressValue = timeProgress;
                        }
                    }
                    
                    float currentProgress = progress->load(std::memory_order_acquire);
                    float finalProgress = (std::min)(94.0f, (std::max)(currentProgress, progressValue));
                    finalProgress = (std::max)(50.0f, finalProgress);
                    progress->store(finalProgress, std::memory_order_relaxed);
                    lastProgressValue = finalProgress;
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
            else
            {
                while (!extractionDone)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
            }
            
            extractionThread.join();
            
            if (progress)
            {

                unsigned long long lastCheckSize = 0;
                unsigned long long lastCheckFiles = 0;
                int stableCount = 0;
                
                for (int i = 0; i < 20; ++i)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    
                    FileCountInfo checkInfo = CountFilesRecursively(restorePath);
                    unsigned long long currentSize = checkInfo.totalSize;
                    unsigned long long currentFiles = checkInfo.totalFiles;
                    
                    if (currentSize == lastCheckSize && currentFiles == lastCheckFiles)
                    {
                        stableCount++;
                        if (stableCount >= 3)
                        {
                            break;
                        }
                    }
                    else
                    {
                        stableCount = 0;

                        if (estimatedUncompressedSize > 0 && currentSize > 0)
                        {
                            float sizeProgress = (std::min)(95.0f, 50.0f + (float)(currentSize * 45.0 / estimatedUncompressedSize));
                            float currentProgress = progress->load(std::memory_order_acquire);
                            float finalProgress = (std::min)(95.0f, (std::max)(currentProgress, sizeProgress));
                            progress->store(finalProgress, std::memory_order_relaxed);
                        }
                    }
                    
                    lastCheckSize = currentSize;
                    lastCheckFiles = currentFiles;
                }
                
                float currentProgress = progress->load(std::memory_order_acquire);
                if (currentProgress > 95.0f)
                {
                    progress->store(95.0f, std::memory_order_relaxed);
                }
            }
            else
            {

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            
            bool success = extractionSuccess.load(std::memory_order_acquire);
            FileCountInfo extractedInfo = CountFilesRecursively(restorePath);
            if (extractedInfo.totalFiles == 0)
            {
                success = false;
            }
            
            if (progress)
            {
                if (success)
                {

                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    progress->store(100.0f, std::memory_order_relaxed);
                }
                else
                {
                    progress->store(0.0f, std::memory_order_relaxed);
                }
            }
            
            if (isEncrypted)
            {
                DeleteFileW(tempZipPath.c_str());
            }
            
            return success;
        }
    }
    catch (...)
    {
        return false;
    }
}

void CheckAndPerformAutoBackup(const std::vector<std::wstring>& folders, const std::wstring& destinationPath, Preferences& prefs, bool* isBackingUp, std::atomic<float>* progress, std::atomic<float>* currentFileProgress, std::mutex* statusMutex, std::string* currentStatus, std::string* currentFileName, std::string* statusMessage, HWND hwnd)
{
    if (!prefs.autoBackup || folders.empty() || destinationPath.empty())
    {
        return;
    }
    
    if (isBackingUp && *isBackingUp)
    {
        return;
    }
    
    std::time_t now = std::time(nullptr);
    std::time_t intervalSeconds = prefs.BackupInterval * 60 * 60;
    
    if (prefs.lastBackupTime == 0 || (now - prefs.lastBackupTime) >= intervalSeconds)
    {

        prefs.lastBackupTime = now;
        SavePreferences(prefs);
        
        if (isBackingUp)
        {
            *isBackingUp = true;
        }
        if (progress)
        {
            progress->store(0.0f, std::memory_order_relaxed);
        }
        if (currentFileProgress)
        {
            currentFileProgress->store(0.0f, std::memory_order_relaxed);
        }
        if (statusMutex && currentStatus)
        {
            std::lock_guard<std::mutex> lock(*statusMutex);
            *currentStatus = "Sauvegarde automatique en cours...";
        }
        if (currentFileName)
        {
            *currentFileName = "";
        }
        if (statusMessage)
        {
            *statusMessage = Translate(TR::AUTO_Backup_IN_PROGRESS);
        }
        
        Preferences prefsCopy = prefs;
        std::thread BackupThread([folders, destinationPath, prefsCopy, isBackingUp, progress, currentFileProgress, statusMutex, currentStatus, currentFileName, statusMessage, hwnd]() {
            bool success = PerformBackup(folders, destinationPath, prefsCopy, progress, currentFileProgress, statusMutex, currentStatus, currentFileName, nullptr);
            
            std::string savedFileName;
            if (statusMutex && currentFileName)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                savedFileName = *currentFileName;
            }
            
            if (isBackingUp)
            {
                *isBackingUp = false;
            }
            if (progress)
            {
                progress->store(100.0f, std::memory_order_relaxed);
            }
            if (currentFileProgress)
            {
                currentFileProgress->store(100.0f, std::memory_order_relaxed);
            }
            if (statusMutex && currentStatus)
            {
                std::lock_guard<std::mutex> lock(*statusMutex);
                *currentStatus = success ? std::string(Translate(TR::AUTO_Backup_SUCCESS)) : std::string(Translate(TR::AUTO_Backup_ERROR));
            }
            if (statusMessage)
            {
                *statusMessage = success ? Translate(TR::AUTO_Backup_SUCCESS) : Translate(TR::AUTO_Backup_ERROR);
            }
            
            if (success)
            {
                Preferences prefsTemp = prefsCopy;
                prefsTemp.lastBackupTime = std::time(nullptr);
                SavePreferences(prefsTemp);
                
                if (prefsCopy.autoDeleteOldBackups)
                {
                    DeleteAllOldBackups(destinationPath, prefsCopy.maxBackups);
                }
                
                if (hwnd && prefsCopy.showNotifications)
                {
                    std::wstring fileNameW = StringToWString(savedFileName);
                    std::wstring notificationMsg = L"La sauvegarde automatique s'est terminée avec succès!\nFichier: " + fileNameW;
                    ShowTrayNotification(hwnd, L"Backup - Sauvegarde automatique terminée", notificationMsg.c_str());
                }
            }
            else if (hwnd && prefsCopy.showNotifications)
            {
                ShowTrayNotification(hwnd, L"Backup - Erreur", L"Une erreur s'est produite lors de la sauvegarde automatique!");
            }
        });
        BackupThread.detach();
    }
}
