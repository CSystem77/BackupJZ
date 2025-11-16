#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>

void SaveConfiguration(const std::vector<std::wstring>& folders, const std::wstring& destinationPath);
bool LoadConfiguration(std::vector<std::wstring>& folders, std::wstring& destinationPath);
std::wstring GetConfigFilePath();

#endif
