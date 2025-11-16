#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include <vector>
#include <atomic>

std::string XorEncrypt(const std::string& input, const std::string& key);
std::string XorDecrypt(const std::string& cypher, const std::string& key);
std::string Base64Encode(const std::vector<unsigned char>& data);
std::vector<unsigned char> Base64Decode(const std::string& encoded);

void EncryptZipFile(const std::wstring& zipPath, const std::string& key);
void DecryptZipFile(const std::wstring& encryptedPath, const std::wstring& outputPath, const std::string& key, std::atomic<float>* progress = nullptr);

#endif
