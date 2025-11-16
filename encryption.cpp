#include "encryption.h"
#include "file_utils.h"
#include <vector>
#include <algorithm>
#include <windows.h>
#include <fstream>

std::string Base64Encode(const std::vector<unsigned char>& data)
{
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int val = 0, valb = -6;
    
    for (unsigned char c : data)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6)
    {
        encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    while (encoded.size() % 4)
    {
        encoded.push_back('=');
    }
    
    while (!encoded.empty() && encoded.back() == '=')
    {
        encoded.pop_back();
    }
    
    return encoded;
}

std::vector<unsigned char> Base64Decode(const std::string& encoded)
{
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<unsigned char> decoded;
    int val = 0, valb = -8;
    
    std::string padded = encoded;
    while (padded.size() % 4 != 0)
    {
        padded.push_back('=');
    }
    
    for (char c : padded)
    {
        if (c == '=') break;
        
        std::string base64_str(base64_chars);
        size_t pos = base64_str.find(c);
        if (pos == std::string::npos) continue;
        
        int idx = static_cast<int>(pos);
        val = (val << 6) + idx;
        valb += 6;
        
        if (valb >= 0)
        {
            decoded.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    
    return decoded;
}

std::string XorEncrypt(const std::string& input, const std::string& key)
{
    if (key.empty())
    {
        return "";
    }
    
    std::vector<unsigned char> output;
    size_t spos = 0;
    
    for (size_t pos = 0; pos < input.length(); ++pos)
    {
        output.push_back(input[pos] ^ key[spos]);
        ++spos;
        if (spos >= key.length())
        {
            spos = 0;
        }
    }
    
    return Base64Encode(output);
}

std::string XorDecrypt(const std::string& cypher, const std::string& key)
{
    if (key.empty() || cypher.empty())
    {
        return "";
    }
    
    try
    {
        std::vector<unsigned char> decoded = Base64Decode(cypher);
        std::string output;
        size_t spos = 0;
        
        for (size_t pos = 0; pos < decoded.size(); ++pos)
        {
            output += static_cast<char>(decoded[pos] ^ key[spos]);
            ++spos;
            if (spos >= key.length())
            {
                spos = 0;
            }
        }
        
        return output;
    }
    catch (...)
    {
        return "";
    }
}

void EncryptZipFile(const std::wstring& zipPath, const std::string& key)
{
    if (key.empty())
    {
        return;
    }
    
    std::string zipPathStr = WStringToString(zipPath);
    std::ifstream inputFile(zipPathStr, std::ios::binary);
    
    if (!inputFile.is_open())
    {
        return;
    }
    
    std::wstring encryptedPath = zipPath;
    size_t zipPos = encryptedPath.rfind(L".zip");
    if (zipPos != std::wstring::npos)
    {
        encryptedPath.replace(zipPos, 4, L".jz");
    }
    else
    {
        encryptedPath += L".jz";
    }
    std::string encryptedPathStr = WStringToString(encryptedPath);
    std::ofstream outputFile(encryptedPathStr, std::ios::binary);
    
    if (!outputFile.is_open())
    {
        inputFile.close();
        return;
    }
    
    const size_t BUFFER_SIZE = 1024 * 1024;
    std::vector<char> buffer(BUFFER_SIZE);
    size_t keyPos = 0;
    
    while (inputFile.read(buffer.data(), BUFFER_SIZE) || inputFile.gcount() > 0)
    {
        size_t bytesRead = static_cast<size_t>(inputFile.gcount());
        
        for (size_t i = 0; i < bytesRead; ++i)
        {
            buffer[i] ^= key[keyPos];
            ++keyPos;
            if (keyPos >= key.length())
            {
                keyPos = 0;
            }
        }
        
        outputFile.write(buffer.data(), bytesRead);
        
        if (bytesRead < BUFFER_SIZE)
        {
            break;
        }
    }
    
    inputFile.close();
    outputFile.close();
    
    WIN32_FIND_DATAW findData;
    HANDLE hFile = FindFirstFileW(encryptedPath.c_str(), &findData);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
        DeleteFileW(zipPath.c_str());
    }
}

void DecryptZipFile(const std::wstring& encryptedPath, const std::wstring& outputPath, const std::string& key, std::atomic<float>* progress)
{
    if (key.empty())
    {
        return;
    }
    
    std::string encryptedPathStr = WStringToString(encryptedPath);
    std::ifstream inputFile(encryptedPathStr, std::ios::binary);
    
    if (!inputFile.is_open())
    {
        return;
    }
    
    unsigned long long totalSize = GetFileSize(encryptedPath);
    unsigned long long processedSize = 0;
    
    if (progress)
    {
        progress->store(0.0f, std::memory_order_relaxed);
    }
    
    std::string outputPathStr = WStringToString(outputPath);
    std::ofstream outputFile(outputPathStr, std::ios::binary);
    
    if (!outputFile.is_open())
    {
        inputFile.close();
        return;
    }
    
    const size_t BUFFER_SIZE = 1024 * 1024;
    std::vector<char> buffer(BUFFER_SIZE);
    size_t keyPos = 0;
    
    while (inputFile.read(buffer.data(), BUFFER_SIZE) || inputFile.gcount() > 0)
    {
        size_t bytesRead = static_cast<size_t>(inputFile.gcount());
        
        for (size_t i = 0; i < bytesRead; ++i)
        {
            buffer[i] ^= key[keyPos];
            ++keyPos;
            if (keyPos >= key.length())
            {
                keyPos = 0;
            }
        }
        
        outputFile.write(buffer.data(), bytesRead);
        
        processedSize += bytesRead;
        if (progress)
        {
            if (totalSize > 0)
            {
                float progressValue = (std::min)(95.0f, (float)(processedSize * 100.0 / totalSize));
                progress->store(progressValue, std::memory_order_relaxed);
            }
            else if (processedSize > 0)
            {

                unsigned long long estimatedSize = (std::max)(10ULL * 1024 * 1024, processedSize);
                float progressValue = (std::min)(95.0f, (float)(processedSize * 100.0 / estimatedSize));
                progress->store(progressValue, std::memory_order_relaxed);
            }
        }
        
        if (bytesRead < BUFFER_SIZE)
        {
            break;
        }
    }
    
    inputFile.close();
    outputFile.close();
    
    if (progress)
    {
        progress->store(100.0f, std::memory_order_relaxed);
    }
}
