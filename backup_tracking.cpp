#include "Backup_tracking.h"
#include "file_utils.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

std::wstring GetDataDirectoryPath()
{
    wchar_t exePath[MAX_PATH];
    DWORD pathLen = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    
    if (pathLen > 0 && pathLen < MAX_PATH)
    {
        std::wstring exePathStr = exePath;
        size_t lastSlash = exePathStr.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos)
        {
            std::wstring dataPath = exePathStr.substr(0, lastSlash + 1);
            dataPath += L"data";
            return dataPath;
        }
    }
    
    return L"data";
}

std::wstring GetBaseNameFromBackupFile(const std::wstring& BackupFilename)
{

    size_t lastUnderscore = BackupFilename.find_last_of(L'_');
    if (lastUnderscore == std::wstring::npos)
    {

        size_t lastDot = BackupFilename.find_last_of(L'.');
        if (lastDot != std::wstring::npos)
        {
            return BackupFilename.substr(0, lastDot);
        }
        return BackupFilename;
    }
    
    std::wstring beforeLastUnderscore = BackupFilename.substr(0, lastUnderscore);
    
    size_t secondLastUnderscore = beforeLastUnderscore.find_last_of(L'_');
    if (secondLastUnderscore != std::wstring::npos)
    {

        std::wstring potentialDate = beforeLastUnderscore.substr(secondLastUnderscore + 1);
        if (potentialDate.length() == 8)
        {
            bool isDate = true;
            for (wchar_t c : potentialDate)
            {
                if (c < L'0' || c > L'9')
                {
                    isDate = false;
                    break;
                }
            }
            if (isDate)
            {

                return beforeLastUnderscore.substr(0, secondLastUnderscore);
            }
        }
    }
    
    size_t lastDot = BackupFilename.find_last_of(L'.');
    if (lastDot != std::wstring::npos)
    {
        return BackupFilename.substr(0, lastDot);
    }
    return BackupFilename;
}

std::wstring GetTrackingFilePath(const std::wstring& baseName)
{
    std::wstring dataDir = GetDataDirectoryPath();
    
    CreateDirectoryW(dataDir.c_str(), nullptr);
    
    std::wstring safeBaseName = baseName;
    for (size_t i = 0; i < safeBaseName.length(); i++)
    {
        wchar_t c = safeBaseName[i];
        if (c == L'\\' || c == L'/' || c == L':' || c == L'*' || c == L'?' || c == L'"' || c == L'<' || c == L'>' || c == L'|')
        {
            safeBaseName[i] = L'_';
        }
    }
    
    return dataDir + L"\\" + safeBaseName + L".csv";
}

void RegisterBackupFile(const std::wstring& BackupFilename, const std::wstring& destinationPath)
{
    std::wstring baseName = GetBaseNameFromBackupFile(BackupFilename);
    std::wstring trackingPath = GetTrackingFilePath(baseName);
    std::string trackingPathStr = WStringToString(trackingPath);
    
    std::wstring fullPath = destinationPath;
    if (fullPath.back() != L'\\' && fullPath.back() != L'/')
    {
        fullPath += L"\\";
    }
    fullPath += BackupFilename;
    
    std::time_t fileDate = ExtractDateFromFilename(BackupFilename);
    std::tm timeinfo = {0};
    if (fileDate != 0)
    {
        localtime_s(&timeinfo, &fileDate);
    }
    else
    {

        std::time_t now = std::time(nullptr);
        localtime_s(&timeinfo, &now);
    }
    
    std::ostringstream dateStream;
    dateStream << std::setfill('0') << std::setw(2) << timeinfo.tm_mday
               << "/" << std::setw(2) << (timeinfo.tm_mon + 1)
               << "/" << std::setw(4) << (timeinfo.tm_year + 1900);
    std::string dateStr = dateStream.str();
    
    std::ostringstream timeStream;
    timeStream << std::setfill('0') << std::setw(2) << timeinfo.tm_hour
               << ":" << std::setw(2) << timeinfo.tm_min
               << ":" << std::setw(2) << timeinfo.tm_sec;
    std::string timeStr = timeStream.str();
    
    std::vector<std::string> existingLines;
    bool hasHeader = false;
    std::ifstream inFile(trackingPathStr);
    if (inFile.is_open())
    {
        std::string line;
        bool firstLine = true;
        while (std::getline(inFile, line))
        {
            if (!line.empty())
            {
                if (firstLine && line.find("Date;Heure") == 0)
                {
                    hasHeader = true;
                    firstLine = false;
                    continue;
                }
                firstLine = false;
                
                size_t lastSemicolon = line.find_last_of(';');
                if (lastSemicolon != std::string::npos)
                {
                    std::string existingPath = line.substr(lastSemicolon + 1);
                    if (existingPath == WStringToString(fullPath))
                    {
                        inFile.close();
                        return;
                    }
                }
                existingLines.push_back(line);
            }
        }
        inFile.close();
    }
    
    std::string csvLine = dateStr + ";" + timeStr + ";" + WStringToString(BackupFilename) + ";" + WStringToString(fullPath);
    
    existingLines.push_back(csvLine);
    
    std::ofstream outFile(trackingPathStr, std::ios::trunc);
    if (outFile.is_open())
    {

        outFile << "Date;Heure;Nom du Fichier;Lien du fichier\n";
        
        for (const auto& line : existingLines)
        {
            outFile << line << "\n";
        }
        outFile.close();
    }
}

void RemoveBackupFromTracking(const std::wstring& BackupFilename, const std::wstring& destinationPath)
{
    std::wstring baseName = GetBaseNameFromBackupFile(BackupFilename);
    std::wstring trackingPath = GetTrackingFilePath(baseName);
    std::string trackingPathStr = WStringToString(trackingPath);
    
    std::wstring fullPath = destinationPath;
    if (fullPath.back() != L'\\' && fullPath.back() != L'/')
    {
        fullPath += L"\\";
    }
    fullPath += BackupFilename;
    std::string fullPathStr = WStringToString(fullPath);
    
    std::vector<std::string> existingLines;
    bool hasHeader = false;
    std::ifstream inFile(trackingPathStr);
    if (inFile.is_open())
    {
        std::string line;
        bool firstLine = true;
        while (std::getline(inFile, line))
        {
            if (!line.empty())
            {
                if (firstLine && line.find("Date;Heure") == 0)
                {
                    hasHeader = true;
                    firstLine = false;
                    existingLines.push_back(line);
                    continue;
                }
                firstLine = false;
                
                size_t lastSemicolon = line.find_last_of(';');
                if (lastSemicolon != std::string::npos)
                {
                    std::string existingPath = line.substr(lastSemicolon + 1);
                    if (existingPath != fullPathStr)
                    {
                        existingLines.push_back(line);
                    }
                }
                else
                {
                    existingLines.push_back(line);
                }
            }
        }
        inFile.close();
    }
    
    std::ofstream outFile(trackingPathStr, std::ios::trunc);
    if (outFile.is_open())
    {
        for (const auto& line : existingLines)
        {
            outFile << line << "\n";
        }
        outFile.close();
    }
}

std::vector<std::wstring> GetBackupFilesFromTracking(const std::wstring& baseName, const std::wstring& destinationPath)
{
    (void)destinationPath;
    std::vector<std::wstring> files;
    std::wstring trackingPath = GetTrackingFilePath(baseName);
    std::string trackingPathStr = WStringToString(trackingPath);
    
    std::ifstream inFile(trackingPathStr);
    if (inFile.is_open())
    {
        std::string line;
        bool firstLine = true;
        while (std::getline(inFile, line))
        {
            if (!line.empty())
            {

                if (firstLine && line.find("Date;Heure") == 0)
                {
                    firstLine = false;
                    continue;
                }
                firstLine = false;
                
                size_t lastSemicolon = line.find_last_of(';');
                if (lastSemicolon != std::string::npos)
                {
                    std::string filePath = line.substr(lastSemicolon + 1);
                    if (!filePath.empty())
                    {
                        files.push_back(StringToWString(filePath));
                    }
                }
            }
        }
        inFile.close();
    }
    
    return files;
}

void DeleteAllOldBackups(const std::wstring& destinationPath, int maxBackups)
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

                            WIN32_FIND_DATAW fileData;
                            HANDLE hFile = FindFirstFileW(filePath.c_str(), &fileData);
                            if (hFile == INVALID_HANDLE_VALUE)
                            {

                                size_t lastSlash = filePath.find_last_of(L"\\/");
                                std::wstring filename = (lastSlash != std::wstring::npos) ? 
                                    filePath.substr(lastSlash + 1) : filePath;
                                RemoveBackupFromTracking(filename, destinationPath);
                                continue;
                            }
                            
                            FindClose(hFile);
                            
                            size_t lastSlash = filePath.find_last_of(L"\\/");
                            std::wstring filename = (lastSlash != std::wstring::npos) ? 
                                filePath.substr(lastSlash + 1) : filePath;
                            
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

std::vector<std::wstring> GetAllBackups(const std::wstring& destinationPath)
{
    std::vector<std::wstring> allBackups;
    
    if (destinationPath.empty())
    {
        return allBackups;
    }
    
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
                    
                    for (const auto& BackupFile : BackupFiles)
                    {

                        WIN32_FIND_DATAW fileData;
                        HANDLE hFile = FindFirstFileW(BackupFile.c_str(), &fileData);
                        if (hFile != INVALID_HANDLE_VALUE)
                        {
                            FindClose(hFile);
                            allBackups.push_back(BackupFile);
                        }
                    }
                }
            }
        } while (FindNextFileW(hFind, &findData) != 0);
        FindClose(hFind);
    }
    
    return allBackups;
}
