#include "config.h"
#include "file_utils.h"
#include "encryption.h"
#include <windows.h>
#include <fstream>
#include <sstream>

std::wstring GetConfigFilePath()
{
    wchar_t exePath[MAX_PATH];
    DWORD pathLen = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    
    if (pathLen > 0 && pathLen < MAX_PATH)
    {
        std::wstring exePathStr = exePath;

        size_t lastSlash = exePathStr.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos)
        {
            std::wstring configPath = exePathStr.substr(0, lastSlash + 1);
            configPath += L"config.ini.jz";
            return configPath;
        }
    }
    
    return L"config.ini.jz";
}

void SaveConfiguration(const std::vector<std::wstring>& folders, const std::wstring& destinationPath)
{
    std::wstring configPath = GetConfigFilePath();
    std::string configPathStr = WStringToString(configPath);
    
    std::string content;
    content += "DESTINATION:" + WStringToString(destinationPath) + "\n";
    content += "FOLDERS:\n";
    for (const auto& folder : folders)
    {
        content += WStringToString(folder) + "\n";
    }
    
    const std::string key = "Backupjz";
    std::string encrypted = XorEncrypt(content, key);
    
    std::ofstream file(configPathStr, std::ios::out | std::ios::trunc | std::ios::binary);
    if (file.is_open())
    {
        file << encrypted;
        file.close();
    }
}

bool LoadConfiguration(std::vector<std::wstring>& folders, std::wstring& destinationPath)
{
    std::wstring configPath = GetConfigFilePath();
    std::string configPathStr = WStringToString(configPath);
    std::ifstream file(configPathStr, std::ios::binary);
    
    if (!file.is_open())
    {
        return false;
    }
    
    std::string encryptedContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    if (encryptedContent.empty())
    {
        return false;
    }
    
    const std::string key = "Backupjz";
    std::string content = XorDecrypt(encryptedContent, key);
    
    if (content.empty() || (content.find("DESTINATION:") == std::string::npos && content.find("FOLDERS:") == std::string::npos))
    {

        if (encryptedContent.find("DESTINATION:") != std::string::npos || encryptedContent.find("FOLDERS:") != std::string::npos)
        {
            content = encryptedContent;
        }
        else
        {
            return false;
        }
    }
    
    folders.clear();
    destinationPath.clear();
    
    std::istringstream stream(content);
    std::string line;
    bool readingFolders = false;
    
    while (std::getline(stream, line))
    {

        if (!line.empty())
        {
            size_t first = line.find_first_not_of(" \t\r\n");
            size_t last = line.find_last_not_of(" \t\r\n");
            if (first != std::string::npos && last != std::string::npos)
            {
                line = line.substr(first, last - first + 1);
            }
            else
            {
                line.clear();
            }
        }
        
        if (line.empty()) continue;
        
        if (line.find("DESTINATION:") == 0)
        {
            std::string dest = line.substr(12);

            size_t first = dest.find_first_not_of(" \t");
            size_t last = dest.find_last_not_of(" \t");
            if (first != std::string::npos && last != std::string::npos)
            {
                dest = dest.substr(first, last - first + 1);
                destinationPath = StringToWString(dest);
            }
        }

        else if (line == "FOLDERS:")
        {
            readingFolders = true;
        }

        else if (readingFolders)
        {
            folders.push_back(StringToWString(line));
        }
    }
    
    return true;
}
