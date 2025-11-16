#include "file_utils.h"
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <comdef.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <filesystem>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

std::wstring GetCurrentDateTimeString()
{
    std::time_t now = std::time(nullptr);
    std::tm timeinfo;
    localtime_s(&timeinfo, &now);
    
    std::wostringstream oss;
    oss << std::setfill(L'0') << std::setw(4) << (timeinfo.tm_year + 1900)
        << std::setw(2) << (timeinfo.tm_mon + 1)
        << std::setw(2) << timeinfo.tm_mday
        << L"_"
        << std::setw(2) << timeinfo.tm_hour
        << std::setw(2) << timeinfo.tm_min
        << std::setw(2) << timeinfo.tm_sec;
    
    return oss.str();
}

std::string GetFormattedDateTimeString()
{
    std::time_t now = std::time(nullptr);
    std::tm timeinfo;
    localtime_s(&timeinfo, &now);
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << timeinfo.tm_mday
        << "/" << std::setw(2) << (timeinfo.tm_mon + 1)
        << "/" << std::setw(4) << (timeinfo.tm_year + 1900)
        << " - "
        << std::setw(2) << timeinfo.tm_hour
        << ":" << std::setw(2) << timeinfo.tm_min
        << ":" << std::setw(2) << timeinfo.tm_sec;
    
    return oss.str();
}

std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

std::wstring StringToWString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::time_t ExtractDateFromFilename(const std::wstring& filename)
{

    size_t lastUnderscore = filename.find_last_of(L'_');
    if (lastUnderscore == std::wstring::npos)
    {
        return 0;
    }
    
    std::wstring afterLastUnderscore = filename.substr(lastUnderscore + 1);
    
    size_t firstDot = afterLastUnderscore.find_first_of(L'.');
    if (firstDot == std::wstring::npos)
    {
        return 0;
    }
    
    std::wstring dateTimePart = afterLastUnderscore.substr(0, firstDot);
    
    if (dateTimePart.length() != 15)
    {
        return 0;
    }
    
    size_t dateUnderscore = dateTimePart.find(L'_');
    if (dateUnderscore == std::wstring::npos || dateUnderscore != 8)
    {
        return 0;
    }
    
    std::wstring dateStr = dateTimePart.substr(0, 8);
    std::wstring timeStr = dateTimePart.substr(9, 6);
    
    for (wchar_t c : dateStr)
    {
        if (c < L'0' || c > L'9')
        {
            return 0;
        }
    }
    for (wchar_t c : timeStr)
    {
        if (c < L'0' || c > L'9')
        {
            return 0;
        }
    }
    
    try
    {
        int year = std::stoi(dateStr.substr(0, 4));
        int month = std::stoi(dateStr.substr(4, 2));
        int day = std::stoi(dateStr.substr(6, 2));
        int hour = std::stoi(timeStr.substr(0, 2));
        int minute = std::stoi(timeStr.substr(2, 2));
        int second = std::stoi(timeStr.substr(4, 2));
        
        if (month < 1 || month > 12 || day < 1 || day > 31 ||
            hour > 23 || minute > 59 || second > 59)
        {
            return 0;
        }
        
        std::tm timeinfo = {0};
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hour;
        timeinfo.tm_min = minute;
        timeinfo.tm_sec = second;
        timeinfo.tm_isdst = -1;
        
        return std::mktime(&timeinfo);
    }
    catch (...)
    {
        return 0;
    }
}

bool FileExists(const std::wstring& filePath)
{
    WIN32_FIND_DATAW findData;
    HANDLE hFile = FindFirstFileW(filePath.c_str(), &findData);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    FindClose(hFile);
    return true;
}

unsigned long long GetFileSize(const std::wstring& filePath)
{
    WIN32_FIND_DATAW findData;
    HANDLE hFile = FindFirstFileW(filePath.c_str(), &findData);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    FindClose(hFile);
    
    ULARGE_INTEGER fileSize;
    fileSize.LowPart = findData.nFileSizeLow;
    fileSize.HighPart = findData.nFileSizeHigh;
    return fileSize.QuadPart;
}

FileCountInfo CountFilesRecursively(const std::wstring& folderPath)
{
    FileCountInfo info;
    std::wstring searchPath = folderPath + L"\\*";
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return info;
    }
    
    do
    {
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
        {
            continue;
        }
        
        std::wstring fullPath = folderPath + L"\\" + findData.cFileName;
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {

            FileCountInfo subInfo = CountFilesRecursively(fullPath);
            info.totalFiles += subInfo.totalFiles;
            info.totalSize += subInfo.totalSize;
        }
        else
        {

            info.totalFiles++;

            ULARGE_INTEGER fileSize;
            fileSize.LowPart = findData.nFileSizeLow;
            fileSize.HighPart = findData.nFileSizeHigh;
            info.totalSize += fileSize.QuadPart;
        }
    } while (FindNextFileW(hFind, &findData) != 0);
    
    FindClose(hFind);
    return info;
}

std::wstring SelectFolder()
{
    std::wstring result;
    IFileDialog* pfd = nullptr;
    
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
        {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }
        
        if (SUCCEEDED(pfd->Show(nullptr)))
        {
            IShellItem* psi;
            if (SUCCEEDED(pfd->GetResult(&psi)))
            {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
                {
                    result = pszPath;
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
    
    return result;
}

std::wstring SelectDestinationFolder()
{
    return SelectFolder();
}

bool CopyDirectoryRecursive(const std::wstring& sourcePath, const std::wstring& destPath)
{
    namespace fs = std::filesystem;
    
    try
    {
        std::string sourcePathStr = WStringToString(sourcePath);
        std::string destPathStr = WStringToString(destPath);
        
        fs::path source(sourcePathStr);
        fs::path dest(destPathStr);
        
        if (!fs::exists(source))
        {
            return false;
        }
        
        if (!fs::exists(dest))
        {
            fs::create_directories(dest);
        }
        
        if (fs::is_regular_file(source))
        {
            fs::copy_file(source, dest / source.filename(), fs::copy_options::overwrite_existing);
            return true;
        }
        
        if (fs::is_directory(source))
        {
            std::wstring sourceName = source.filename().wstring();
            if (sourceName.empty())
            {
                sourceName = sourcePath.substr(sourcePath.find_last_of(L"\\/") + 1);
            }
            
            fs::path destDir = dest / sourceName;
            fs::create_directories(destDir);
            
            for (const auto& entry : fs::recursive_directory_iterator(source))
            {
                try
                {
                    fs::path relativePath = fs::relative(entry.path(), source);
                    fs::path destFile = destDir / relativePath;
                    
                    if (fs::is_directory(entry.status()))
                    {
                        fs::create_directories(destFile);
                    }
                    else if (fs::is_regular_file(entry.status()))
                    {
                        fs::create_directories(destFile.parent_path());
                        fs::copy_file(entry.path(), destFile, fs::copy_options::overwrite_existing);
                    }
                }
                catch (...)
                {
                    continue;
                }
            }
            
            return true;
        }
        
        return false;
    }
    catch (...)
    {
        return false;
    }
}

bool DeleteDirectoryRecursive(const std::wstring& dirPath)
{
    namespace fs = std::filesystem;
    
    try
    {
        std::string dirPathStr = WStringToString(dirPath);
        fs::path dir(dirPathStr);
        
        if (!fs::exists(dir))
        {
            return true; 
        }
        
        if (fs::is_directory(dir))
        {
            fs::remove_all(dir);
            return true;
        }
        else if (fs::is_regular_file(dir))
        {
            return fs::remove(dir);
        }
        
        return false;
    }
    catch (...)
    {
        return false;
    }
}
