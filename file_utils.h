#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>
#include <ctime>

struct FileCountInfo {
    size_t totalFiles = 0;
    unsigned long long totalSize = 0;
};

std::string WStringToString(const std::wstring& wstr);
std::wstring StringToWString(const std::string& str);

std::wstring GetCurrentDateTimeString();
std::string GetFormattedDateTimeString();
std::time_t ExtractDateFromFilename(const std::wstring& filename);

bool FileExists(const std::wstring& filePath);
unsigned long long GetFileSize(const std::wstring& filePath);
FileCountInfo CountFilesRecursively(const std::wstring& folderPath);
bool CopyDirectoryRecursive(const std::wstring& sourcePath, const std::wstring& destPath);
bool DeleteDirectoryRecursive(const std::wstring& dirPath);

std::wstring SelectFolder();
std::wstring SelectDestinationFolder();

#endif
