
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/gl.h>
#include <tchar.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <comdef.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <thread>
#include <chrono>
#include <map>
#include <cstring>
#include <atomic>
#include <mutex>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "gdiplus.lib")

#include "window.h"
#include <gdiplus.h>
#include "file_utils.h"
#include "preferences.h"
#include "config.h"
#include "encryption.h"
#include "Backup_tracking.h"
#include "Backup.h"
#include "logo_embedded.h"
#include "translation.h"

using namespace Gdiplus;
ULONG_PTR g_gdiplusToken = 0;

int main(int, char**)
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr);
    
    HWND hConsole = GetConsoleWindow();
    if (hConsole != nullptr)
    {
        ShowWindow(hConsole, SW_HIDE);
    }
    
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
    
    HICON hAppIcon = LoadIconFromMemory(g_logoData, g_logoDataSize, 32, 32);
    if (!hAppIcon)
    {
        hAppIcon = (HICON)LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 32, 32, LR_SHARED);
    }
    
    WNDCLASSEXW wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), hAppIcon, nullptr, nullptr, nullptr, L"ImGui Example", hAppIcon };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"BackupJZ", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 100, 100, (int)(800 * main_scale), (int)(600 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceWGL(hwnd, &g_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &g_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    RECT rect;
    ::GetClientRect(hwnd, &rect);
    g_Width = rect.right - rect.left;
    g_Height = rect.bottom - rect.top;

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    CreateTrayIcon(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init();

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::vector<std::wstring> selectedFolders;
    std::wstring destinationFolder;
    static char destinationPathBuffer[512] = "";
    static bool isBackingUp = false;
    static bool isRestoring = false;
    static std::atomic<bool> shouldCancelBackup(false);
    static std::string statusMessage = "";
    static bool showSuccessMessage = false;
    static std::chrono::steady_clock::time_point popupOpenTime;
    static bool popupTimerActive = false;
    static bool needsSave = false;
    static std::atomic<float> BackupProgress(0.0f);
    static std::atomic<float> currentFileProgress(0.0f);
    static std::mutex statusMutex;
    static std::string currentBackupStatus = "";
    static std::string currentFileName = "";
    static bool BackupSuccess = false;

    LoadConfiguration(selectedFolders, destinationFolder);
    if (!destinationFolder.empty())
    {
        std::string destStr = WStringToString(destinationFolder);
        strncpy_s(destinationPathBuffer, destStr.c_str(), sizeof(destinationPathBuffer) - 1);
    }

    Preferences prefs;
    LoadPreferences(prefs);
    static bool preferencesInitialized = false;

    bool done = false;
    while (!done)
    {

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;
        
        LoadPreferences(prefs);
        CheckAndPerformAutoBackup(selectedFolders, destinationFolder, prefs, &isBackingUp, &BackupProgress, &currentFileProgress, &statusMutex, &currentBackupStatus, &currentFileName, &statusMessage, hwnd);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        float menuBarHeight = 0.0f;
        static bool showAbout = false;
        static bool showPreferences = false;
        static bool showAppSettings = false;
        static bool showRestore = false;
        
        if (g_IsMinimizedToTray || ::IsIconic(hwnd))
        {
            if (ImGui::IsPopupOpen("À propos"))
            {
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsPopupOpen("Opération réussie"))
            {
                ImGui::CloseCurrentPopup();
            }
            showPreferences = false;
            showAppSettings = false;
            showRestore = false;
            showAbout = false;
            showSuccessMessage = false;
            
            ImGui::Render();
            ::Sleep(10);
            continue;
        }
        if (ImGui::BeginMainMenuBar())
        {

            if (isBackingUp || isRestoring)
            {
                ImGui::BeginDisabled();
            }
            
            if (ImGui::BeginMenu(Translate(TR::FILE_MENU)))
            {
                if (ImGui::MenuItem((std::string(Translate(TR::Backup_PREFERENCES)) + "###Menu_File_Backup_Preferences").c_str()))
                {
                    showPreferences = true;
                }
                if (ImGui::MenuItem((std::string(Translate(TR::APPLICATION_SETTINGS)) + "###Menu_File_Application_Settings").c_str()))
                {
                    showAppSettings = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem((std::string(Translate(TR::QUIT)) + "###Menu_File_Quit").c_str()))
                {
                    //Seulement si l'application est Fermer classiquement
                    /*ShowWindow(hwnd, SW_HIDE);
                    g_IsMinimizedToTray = true;
                    ShowTrayNotification(hwnd, L"BackupJZ - En arrière-plan", L"L'application continue de fonctionner en arrière-plan. Cliquez sur l'icône pour la restaurer.");*/
					exit(1);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(Translate(TR::RESTORE_MENU)))
            {
                if (ImGui::MenuItem((std::string(Translate(TR::RESTORE_Backup)) + "###Menu_Restore_Backup").c_str()))
                {
                    showRestore = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(Translate(TR::HELP_MENU)))
            {
                if (ImGui::MenuItem((std::string(Translate(TR::ABOUT)) + "###Menu_Help_About").c_str()))
                {
                    showAbout = true;
                }
                ImGui::EndMenu();
            }
            
            if (isBackingUp || isRestoring)
            {
                ImGui::EndDisabled();
            }
            
            menuBarHeight = ImGui::GetWindowHeight();
            ImGui::EndMainMenuBar();
        }

        if (showAbout)
        {
            if (!g_IsMinimizedToTray && !::IsIconic(hwnd))
            {
                ImGui::OpenPopup(Translate(TR::ABOUT));
            }
            showAbout = false;
        }

        if (!g_IsMinimizedToTray && !::IsIconic(hwnd) && ImGui::BeginPopupModal(Translate(TR::ABOUT), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::Text(Translate(TR::ABOUT_TITLE));
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text(Translate(TR::ABOUT_DESCRIPTION));
            ImGui::Spacing();
            ImGui::Text(Translate(TR::ABOUT_DEVELOPED));
            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Button((std::string(Translate(TR::CLOSE)) + "###Popup_About_Close_Button").c_str(), ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        {
            static bool wasPreferencesOpen = false;
            
            if (showPreferences)
            {
                if (g_IsMinimizedToTray || ::IsIconic(hwnd))
                {
                    showPreferences = false;
                }
            }
            
            if (showPreferences && !g_IsMinimizedToTray && !::IsIconic(hwnd))
            {
                ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_Width), static_cast<float>(g_Height)), ImGuiCond_Always);
                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
                if (ImGui::Begin(Translate(TR::Backup_PREFERENCES), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
                {

                    float buttonWidth = 120.0f;
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - ImGui::GetStyle().WindowPadding.x);
                    if (ImGui::Button((std::string(Translate(TR::CLOSE)) + "###Window_Backup_Preferences_Close_Button").c_str(), ImVec2(buttonWidth, 0)))
                    {
                        showPreferences = false;
                    }
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    static bool autoBackup = false;
                    static int BackupInterval = 1;
                    static bool compressOnBackup = true;
                    static bool showNotifications = true;
                    static int maxBackups = 10;
                    static bool autoDeleteOldBackups = false;
                    static bool encryptBackups = false;
                    static char encryptionKey[256] = "";
                    
                    if (!wasPreferencesOpen)
                    {

                        Preferences loadedPrefs;
                        if (LoadPreferences(loadedPrefs))
                        {
                            autoBackup = loadedPrefs.autoBackup;
                            BackupInterval = loadedPrefs.BackupInterval;
                            compressOnBackup = loadedPrefs.compressOnBackup;
                            showNotifications = loadedPrefs.showNotifications;
                            maxBackups = loadedPrefs.maxBackups;
                            autoDeleteOldBackups = loadedPrefs.autoDeleteOldBackups;
                            encryptBackups = loadedPrefs.encryptBackups;
                            if (!loadedPrefs.encryptionKey.empty())
                            {
                                strncpy_s(encryptionKey, loadedPrefs.encryptionKey.c_str(), sizeof(encryptionKey) - 1);
                            }
                            else
                            {
                                encryptionKey[0] = '\0';
                            }

                            prefs = loadedPrefs;
                        }
                    }
                    wasPreferencesOpen = true;
                
                ImGui::Text(Translate(TR::Backup_SETTINGS));
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Checkbox((std::string(Translate(TR::AUTO_Backup)) + "###Checkbox_Auto_Backup").c_str(), &autoBackup);
                if (autoBackup)
                {
                    ImGui::Indent();
                    ImGui::Text(Translate(TR::INTERVAL_HOURS));
                    ImGui::SameLine();
                    ImGui::InputInt("###Input_Backup_Interval", &BackupInterval, 1, 24);
                    if (BackupInterval < 1) BackupInterval = 1;
                    ImGui::Unindent();
                }
                
                ImGui::Spacing();
                ImGui::Checkbox((std::string(Translate(TR::AUTO_COMPRESS)) + "###Checkbox_Auto_Compress").c_str(), &compressOnBackup);
                ImGui::Spacing();
                
                ImGui::Separator();
                ImGui::Spacing();
                
                if (compressOnBackup)
                {
                    ImGui::Text(Translate(TR::SECURITY));
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    ImGui::Checkbox((std::string(Translate(TR::ENCRYPT_BackupS)) + "###Checkbox_Encrypt_Backups").c_str(), &encryptBackups);
                    if (encryptBackups)
                    {
                        ImGui::Indent();
                        ImGui::Text(Translate(TR::ENCRYPTION_KEY));
                        ImGui::InputText("###Input_Encryption_Key", encryptionKey, sizeof(encryptionKey), ImGuiInputTextFlags_Password);
                        if (strlen(encryptionKey) == 0)
                        {
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), Translate(TR::KEY_REQUIRED));
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), Translate(TR::KEY_CONFIGURED));
                        }
                        ImGui::Unindent();
                    }
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                }
                else
                {

                    encryptBackups = false;
                }
                
                ImGui::Text(Translate(TR::MAX_BackupS));
                ImGui::SameLine();
                ImGui::InputInt("###Input_Max_Backups", &maxBackups, 1, 5);
                if (maxBackups < 1) maxBackups = 1;
                
                ImGui::Spacing();
                ImGui::Checkbox((std::string(Translate(TR::AUTO_DELETE_OLD)) + "###Checkbox_Auto_Delete_Old_Backups").c_str(), &autoDeleteOldBackups);
                if (autoDeleteOldBackups)
                {
                    ImGui::Indent();
                    ImGui::Text(Translate(TR::WHEN_MAX_REACHED));
                    ImGui::Unindent();
                }
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Text(Translate(TR::INTERFACE));
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Checkbox((std::string(Translate(TR::SHOW_NOTIFICATIONS)) + "###Checkbox_Show_Notifications").c_str(), &showNotifications);
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Text(Translate(TR::MAINTENANCE));
                ImGui::Separator();
                ImGui::Spacing();
                
                if (ImGui::Button((std::string(Translate(TR::DELETE_OLD_BackupS)) + "###Button_Delete_Old_Backups").c_str(), ImVec2(250, 30)))
                {

                    if (!destinationFolder.empty())
                    {

                        LoadPreferences(prefs);
                        DeleteAllOldBackups(destinationFolder, prefs.maxBackups);
                        statusMessage = Translate(TR::Backup_SUCCESS);
                    }
                    else
                    {
                        statusMessage = Translate(TR::ERROR_NO_DESTINATION_FOLDER);
                    }
                }
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                if (ImGui::Button((std::string(Translate(TR::SAVE)) + "###Button_Save_Backup_Preferences").c_str(), ImVec2(120, 30)))
                {

                    prefs.autoBackup = autoBackup;
                    prefs.BackupInterval = BackupInterval;
                    prefs.compressOnBackup = compressOnBackup;
                    prefs.showNotifications = showNotifications;
                    prefs.maxBackups = maxBackups;
                    prefs.autoDeleteOldBackups = autoDeleteOldBackups;
                    prefs.encryptBackups = encryptBackups;
                    prefs.encryptionKey = std::string(encryptionKey);
                    SavePreferences(prefs);
                    showPreferences = false;
                }
                
                ImGui::SameLine();
                
                if (ImGui::Button((std::string(Translate(TR::CANCEL)) + "###Button_Cancel_Backup_Preferences").c_str(), ImVec2(120, 30)))
                {
                    showPreferences = false;
                }
                
                ImGui::End();
                }
            }
            else
            {

                if (wasPreferencesOpen)
                {
                    wasPreferencesOpen = false;
                }
            }
        }

        {
            static bool wasAppSettingsOpen = false;
            
            if (showAppSettings)
            {
                if (g_IsMinimizedToTray || ::IsIconic(hwnd))
                {
                    showAppSettings = false;
                }
            }
            
            if (showAppSettings && !g_IsMinimizedToTray && !::IsIconic(hwnd))
            {
                ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_Width), static_cast<float>(g_Height)), ImGuiCond_Always);
                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
                if (ImGui::Begin(Translate(TR::APPLICATION_SETTINGS_TITLE), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
                {
                    float buttonWidth = 120.0f;
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - ImGui::GetStyle().WindowPadding.x);
                    if (ImGui::Button((std::string(Translate(TR::CLOSE)) + "###Window_Application_Settings_Close_Button_Top").c_str(), ImVec2(buttonWidth, 0)))
                    {
                        showAppSettings = false;
                    }
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    ImGui::Text(Translate(TR::APPLICATION_SETTINGS_TITLE));
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    ImGui::Text(Translate(TR::APP_SETTINGS_DESCRIPTION));
                    ImGui::Spacing();
                    ImGui::Text(Translate(TR::APP_SETTINGS_Backup_NOTE));
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    ImGui::Text(Translate(TR::LANGUAGE));
                    ImGui::SameLine();
                    static int selectedLanguage = 0;
                    Language currentLang = GetCurrentLanguage();
                    selectedLanguage = (currentLang == Language::English) ? 1 : 0;
                    const char* languages[] = { Translate(TR::LANGUAGE_FRENCH), Translate(TR::LANGUAGE_ENGLISH) };
                    if (ImGui::Combo("###Combo_Language", &selectedLanguage, languages, 2))
                    {
                        Language newLang = (selectedLanguage == 1) ? Language::English : Language::French;
                        SetCurrentLanguage(newLang);
                    }
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    if (ImGui::Button((std::string(Translate(TR::CLOSE)) + "###Window_Application_Settings_Close_Button_Bottom").c_str(), ImVec2(120, 30)))
                    {
                        showAppSettings = false;
                    }
                    
                    ImGui::End();
                }
            }
            else
            {
                if (wasAppSettingsOpen)
                {
                    wasAppSettingsOpen = false;
                }
            }
        }

        {

            float availableHeight = (float)g_Height - menuBarHeight;
            
            ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
            ImGui::SetNextWindowSize(ImVec2((float)g_Width, availableHeight));
            ImGui::Begin(Translate(TR::SELECT_FOLDERS_TO_Backup), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

            ImGui::Text(Translate(TR::SELECT_FOLDERS_TO_Backup));
            ImGui::Separator();
            ImGui::Spacing();

            if (isBackingUp || isRestoring)
            {
                ImGui::BeginDisabled();
            }

            if (ImGui::Button((std::string(Translate(TR::ADD_FOLDER)) + "###Button_Add_Folder").c_str(), ImVec2(200, 30)))
            {
                std::wstring folder = SelectFolder();
                if (!folder.empty())
                {

                    if (std::find(selectedFolders.begin(), selectedFolders.end(), folder) == selectedFolders.end())
                    {
                        selectedFolders.push_back(folder);
                        needsSave = true;
                    }
                }
            }

            ImGui::SameLine();
            if (ImGui::Button((std::string(Translate(TR::REMOVE_SELECTION)) + "###Button_Remove_Last_Folder").c_str(), ImVec2(200, 30)) && !selectedFolders.empty())
            {
                selectedFolders.pop_back();
                needsSave = true;
            }
            
            if (isBackingUp || isRestoring)
            {
                ImGui::EndDisabled();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("%s (%d):", Translate(TR::SELECTED_FOLDERS), (int)selectedFolders.size());
            ImGui::BeginChild("DossiersList", ImVec2(0, 200), true);
            if (isBackingUp || isRestoring)
            {
                ImGui::BeginDisabled();
            }
            for (size_t i = 0; i < selectedFolders.size(); i++)
            {
                std::string folderStr = WStringToString(selectedFolders[i]);
                ImGui::Text("%d. %s", (int)(i + 1), folderStr.c_str());
                ImGui::SameLine();
                std::string buttonId = "X###Button_Remove_Folder_" + std::to_string(i);
                if (ImGui::SmallButton(buttonId.c_str()))
                {
                    selectedFolders.erase(selectedFolders.begin() + i);
                    needsSave = true;
                    break;
                }
            }
            if (selectedFolders.empty())
            {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), Translate(TR::NO_FOLDER_SELECTED));
            }
            if (isBackingUp || isRestoring)
            {
                ImGui::EndDisabled();
            }
            ImGui::EndChild();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text(Translate(TR::DESTINATION_FOLDER));
            if (isBackingUp || isRestoring)
            {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button((std::string(Translate(TR::CHOOSE_DESTINATION)) + "###Button_Select_Destination_Folder").c_str(), ImVec2(250, 30)))
            {
                std::wstring dest = SelectDestinationFolder();
                if (!dest.empty())
                {
                    destinationFolder = dest;
                    std::string destStr = WStringToString(dest);
                    strncpy_s(destinationPathBuffer, destStr.c_str(), sizeof(destinationPathBuffer) - 1);
                    needsSave = true;
                }
            }
            if (isBackingUp || isRestoring)
            {
                ImGui::EndDisabled();
            }

            ImGui::Spacing();
            ImGui::Text("%s %s", Translate(TR::PATH_LABEL), destinationPathBuffer[0] ? destinationPathBuffer : Translate(TR::NO_DESTINATION_SELECTED));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (isBackingUp)
            {
                
            }
            else
            {

                if (ImGui::Button((std::string(Translate(TR::Backup_PREFERENCES)) + "###Button_Open_Backup_Preferences").c_str(), ImVec2(200, 40)))
                {
                    showPreferences = true;
                }

                ImGui::SameLine();
                ImGui::Spacing();
                ImGui::SameLine();

                if (ImGui::Button((std::string(Translate(TR::START_Backup)) + "###Button_Start_Backup").c_str(), ImVec2(250, 40)))
            {
                if (selectedFolders.empty())
                {
                    statusMessage = Translate(TR::Backup_ERROR);
                }
                else if (destinationFolder.empty())
                {
                    statusMessage = Translate(TR::Backup_ERROR);
                }
                else if (!isBackingUp)
                {
                    isBackingUp = true;
                    shouldCancelBackup = false;
                    BackupProgress = 0.0f;
                    currentFileProgress = 0.0f;
                    statusMessage = Translate(TR::Backup_IN_PROGRESS);
                    currentBackupStatus = Translate(TR::INITIALIZING);
                    currentFileName = "";
                    BackupSuccess = false;

                    LoadPreferences(prefs);
                    
                    std::vector<std::wstring> foldersCopy = selectedFolders;
                    std::wstring destCopy = destinationFolder;
                    Preferences prefsCopy = prefs;
                    
                    std::thread BackupThread([foldersCopy, destCopy, prefsCopy, hwnd]() {
                        bool success = PerformBackup(foldersCopy, destCopy, prefsCopy, &BackupProgress, &currentFileProgress, &statusMutex, &currentBackupStatus, &currentFileName, &shouldCancelBackup);
                        
                        std::string savedFileName;
                        {
                            std::lock_guard<std::mutex> lock(statusMutex);
                            savedFileName = currentFileName;
                            BackupSuccess = success;
                            isBackingUp = false;
                            
                            if (shouldCancelBackup.load())
                            {
                                statusMessage = Translate(TR::Backup_CANCELLED);
                            }
                            else if (success)
                            {
                                statusMessage = Translate(TR::Backup_SUCCESS);
                                showSuccessMessage = true;
                                Preferences prefsTemp = prefsCopy;
                                prefsTemp.lastBackupTime = std::time(nullptr);
                                SavePreferences(prefsTemp);
                                
                                if (prefsCopy.autoDeleteOldBackups)
                                {
                                    DeleteAllOldBackups(destCopy, prefsCopy.maxBackups);
                                }
                                
                                std::wstring fileNameW = StringToWString(savedFileName);
                                std::wstring notificationMsg = L"La sauvegarde s'est terminée avec succès!";//\nFichier: " + fileNameW;
                                if (prefsCopy.showNotifications)
                                {
                                    ShowTrayNotification(hwnd, L"Backup JZ- Sauvegarde terminée", notificationMsg.c_str());
                                }
                            }
                            else
                            {
                                statusMessage = Translate(TR::Backup_ERROR);
                                if (prefsCopy.showNotifications)
                                {
                                    ShowTrayNotification(hwnd, L"Backup JZ- Erreur", L"Une erreur s'est produite lors de la sauvegarde!");
                                }
                            }
                            BackupProgress = 100.0f;
                            currentFileProgress = 100.0f;
                        }
                    });
                    BackupThread.detach();
                }
            }
            }

            ImGui::Spacing();

            if (isBackingUp)
            {
                float progress = BackupProgress.load(std::memory_order_acquire);
                float fileProgress = currentFileProgress.load(std::memory_order_acquire);
                
                char progressText[32];
                snprintf(progressText, sizeof(progressText), "%.1f%%", progress);
                ImGui::Text(Translate(TR::GLOBAL_PROGRESS_LABEL));
                ImGui::ProgressBar(progress / 100.0f, ImVec2(-1, 0), progressText);
                
                ImGui::Spacing();
                
                char fileProgressText[32];
                snprintf(fileProgressText, sizeof(fileProgressText), "%.1f%%", fileProgress);
                {
                    std::lock_guard<std::mutex> lock(statusMutex);
                    if (!currentFileName.empty())
                    {
                        ImGui::Text("%s %s", Translate(TR::CURRENT_FILE_LABEL), currentFileName.c_str());
                    }
                    else
                    {
                        ImGui::Text(Translate(TR::CURRENT_FILE_LABEL));
                    }
                }
                ImGui::ProgressBar(fileProgress / 100.0f, ImVec2(-1, 0), fileProgressText);
                
                ImGui::Spacing();
                
                {
                    std::lock_guard<std::mutex> lock(statusMutex);
                    if (!currentBackupStatus.empty())
                    {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), currentBackupStatus.c_str());
                    }
                }
            }

            ImGui::Spacing();

            if (!statusMessage.empty())
            {
                ImVec4 color = (statusMessage.find("Erreur") != std::string::npos) ? 
                    ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                ImGui::TextColored(color, statusMessage.c_str());
            }

            ImGui::End();
        }

        if (showRestore)
        {
            if (g_IsMinimizedToTray || ::IsIconic(hwnd))
            {
                showRestore = false;
            }
        }
        
        if (showRestore && !g_IsMinimizedToTray && !::IsIconic(hwnd))
        {
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(g_Width), static_cast<float>(g_Height)), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            if (ImGui::Begin(Translate(TR::RESTORE_Backup_TITLE), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
            {
                static int selectedBackupIndex = -1;
                static std::vector<std::wstring> allBackups;
                static std::vector<std::wstring> BackupNames;
                static bool BackupsLoaded = false;
                static std::atomic<float> restoreProgress(0.0f);
                static std::string restoreStatusMessage = "";
                
                float buttonWidth = 120.0f;
                
                if (isRestoring)
                {
                    ImGui::BeginDisabled();
                }
                
                if (!BackupsLoaded || ImGui::Button((std::string(Translate(TR::REFRESH)) + "###Button_Refresh_Backups").c_str(), ImVec2(buttonWidth, 0)))
                {
                    allBackups = GetAllBackups(destinationFolder);
                    BackupNames.clear();
                    for (const auto& Backup : allBackups)
                    {
                        size_t lastSlash = Backup.find_last_of(L"\\/");
                        std::wstring name = (lastSlash != std::wstring::npos) ? 
                            Backup.substr(lastSlash + 1) : Backup;
                        BackupNames.push_back(name);
                    }
                    BackupsLoaded = true;
                    selectedBackupIndex = -1;
                }
                
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - ImGui::GetStyle().WindowPadding.x);
                if (ImGui::Button((std::string(Translate(TR::CLOSE)) + "###Window_Restore_Close_Button").c_str(), ImVec2(buttonWidth, 0)))
                {
                    if (!isRestoring)
                    {
                        showRestore = false;
                    }
                }
                
                if (isRestoring)
                {
                    ImGui::EndDisabled();
                }
                
                ImGui::Spacing();
                ImGui::Text(Translate(TR::AVAILABLE_BackupS));
                ImGui::Separator();
                
                if (BackupNames.empty())
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), Translate(TR::NO_Backup_FOUND));
                    if (destinationFolder.empty())
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), Translate(TR::Backup_ERROR));
                    }
                }
                else
                {
                    ImGui::BeginChild("BackupList", ImVec2(0, 270), true);
                    if (isRestoring)
                    {
                        ImGui::BeginDisabled();
                    }
                    for (size_t i = 0; i < BackupNames.size(); i++)
                    {
                        std::string nameStr = WStringToString(BackupNames[i]);
                        if (ImGui::Selectable(nameStr.c_str(), selectedBackupIndex == static_cast<int>(i)))
                        {
                            selectedBackupIndex = static_cast<int>(i);
                        }
                    }
                    if (isRestoring)
                    {
                        ImGui::EndDisabled();
                    }
                    ImGui::EndChild();
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    if (selectedBackupIndex >= 0 && selectedBackupIndex < static_cast<int>(allBackups.size()))
                    {
                        std::string selectedName = WStringToString(BackupNames[selectedBackupIndex]);
                        ImGui::Text("%s: %s", Translate(TR::SELECTED_Backup), selectedName.c_str());
                        
                        std::wstring selectedBackupPath = allBackups[selectedBackupIndex];
                        bool isEncrypted = (selectedBackupPath.length() > 3 && 
                                           selectedBackupPath.substr(selectedBackupPath.length() - 3) == L".jz");
                        
                        if (isEncrypted)
                        {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), Translate(TR::ENCRYPTED_Backup_DETECTED));
                            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), Translate(TR::ENCRYPTION_KEY_WARNING));
                        }
                        
                        ImGui::Spacing();
                        ImGui::Text(Translate(TR::RESTORE_FOLDER));
                        static char restorePathBuffer[512] = "";
                        if (isRestoring)
                        {
                            ImGui::BeginDisabled();
                        }
                        ImGui::InputText("###Input_Restore_Path", restorePathBuffer, sizeof(restorePathBuffer));
                        ImGui::SameLine();
                        if (ImGui::Button((std::string(Translate(TR::BROWSE)) + "###Button_Browse_Restore_Path").c_str()))
                        {
                            BROWSEINFOW bi = { 0 };
                            bi.lpszTitle = L"Sélectionner le dossier de restauration";
                            LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
                            if (pidl != nullptr)
                            {
                                wchar_t path[MAX_PATH];
                                if (SHGetPathFromIDListW(pidl, path))
                                {
                                    std::string pathStr = WStringToString(path);
                                    strncpy_s(restorePathBuffer, pathStr.c_str(), sizeof(restorePathBuffer) - 1);
                                }
                                CoTaskMemFree(pidl);
                            }
                        }
                        if (isRestoring)
                        {
                            ImGui::EndDisabled();
                        }
                        
                        ImGui::Spacing();
                        
                        if (isRestoring)
                        {
                            ImGui::Text(Translate(TR::RESTORING_IN_PROGRESS));
                            ImGui::Spacing();
                            
                            float progressValue = restoreProgress.load();
                            if (progressValue < 0.95f && progressValue > 0.0f)
                            {

                                static auto lastTime = std::chrono::steady_clock::now();
                                auto currentTime = std::chrono::steady_clock::now();
                                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
                                
                                if (elapsed > 50)
                                {
                                    float progressSpeed = 0.015f;
                                    progressValue += progressSpeed;
                                    if (progressValue > 0.95f) progressValue = 0.95f;
                                    restoreProgress.store(progressValue);
                                    lastTime = currentTime;
                                }
                            }
                            
                            ImGui::ProgressBar(progressValue, ImVec2(-1, 30), "");
                            
                            if (!restoreStatusMessage.empty())
                            {
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), restoreStatusMessage.c_str());
                            }
                        }
                        else
                        {
                            if (ImGui::Button((std::string(Translate(TR::RESTORE)) + "###Button_Start_Restore").c_str(), ImVec2(200, 30)))
                            {
                                if (strlen(restorePathBuffer) > 0)
                                {
                                    std::wstring restorePath = StringToWString(std::string(restorePathBuffer));
                                    std::wstring BackupPath = allBackups[selectedBackupIndex];
                                    
                                    isRestoring = true;
                                    restoreProgress.store(0.0f);
                                    restoreStatusMessage = Translate(TR::STARTING_RESTORE);
                                    
                                    Preferences prefsRestore;
                                    LoadPreferences(prefsRestore);
                                    
                                    std::thread restoreThread([BackupPath, restorePath, hwnd, prefsRestore]() {
                                        try
                                        {
                                            restoreStatusMessage = Translate(TR::PREPARING_RESTORE);
                                            restoreProgress.store(0.0f);
                                            
                                            bool success = RestoreBackup(BackupPath, restorePath, &restoreProgress, &restoreStatusMessage);
                                            
                                            size_t lastSlash = BackupPath.find_last_of(L"\\/");
                                            std::wstring BackupFileName = (lastSlash != std::wstring::npos) ? 
                                                BackupPath.substr(lastSlash + 1) : BackupPath;
                                            
                                            if (success)
                                            {
                                                restoreStatusMessage = Translate(TR::RESTORE_SUCCESS);
                                                restoreProgress.store(100.0f);
                                                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                                
                                                statusMessage = Translate(TR::RESTORE_SUCCESS);
                                                showSuccessMessage = true;
                                                memset(restorePathBuffer, 0, sizeof(restorePathBuffer));
                                                
                                                if (prefsRestore.showNotifications)
                                                {
                                                    std::wstring notificationMsg = L"La restauration s'est terminée avec succès!\nFichier: " + BackupFileName;
                                                    ShowTrayNotification(hwnd, L"Backup JZ- Restauration terminée", notificationMsg.c_str());
                                                }
                                            }
                                            else
                                            {
                                                restoreStatusMessage = Translate(TR::RESTORE_ERROR);
                                                restoreProgress.store(0.0f);
                                                statusMessage = Translate(TR::RESTORE_ERROR);
                                                
                                                if (prefsRestore.showNotifications)
                                                {
                                                    ShowTrayNotification(hwnd, L"Backup JZ- Erreur", L"Une erreur s'est produite lors de la restauration!");
                                                }
                                            }
                                        }
                                        catch (const std::exception& e)
                                        {
                                            restoreStatusMessage = Translate(TR::RESTORE_ERROR);
                                            restoreProgress.store(0.0f);
                                            statusMessage = Translate(TR::RESTORE_ERROR);
                                            
                                            if (prefsRestore.showNotifications)
                                            {
                                                ShowTrayNotification(hwnd, L"Backup JZ- Erreur", L"Une exception s'est produite lors de la restauration!");
                                            }
                                        }
                                        catch (...)
                                        {
                                            restoreStatusMessage = Translate(TR::RESTORE_ERROR);
                                            restoreProgress.store(0.0f);
                                            statusMessage = Translate(TR::RESTORE_ERROR);
                                            
                                            if (prefsRestore.showNotifications)
                                            {
                                                ShowTrayNotification(hwnd, L"Backup JZ- Erreur", L"Une erreur inconnue s'est produite lors de la restauration!");
                                            }
                                        }
                                        
                                        isRestoring = false;
                                    });
                                    restoreThread.detach();
                                }
                                else
                                {
                                    statusMessage = Translate(TR::ERROR_NO_RESTORE_FOLDER);
                                }
                            }
                        }
                    }
                }
                
                ImGui::End();
            }
        }

        if (showSuccessMessage)
        {
            if (!g_IsMinimizedToTray && !::IsIconic(hwnd))
            {
                ImGui::OpenPopup(Translate(TR::OPERATION_SUCCESS));
                popupOpenTime = std::chrono::steady_clock::now();
                popupTimerActive = true;
            }
            showSuccessMessage = false;
        }

        if (!g_IsMinimizedToTray && !::IsIconic(hwnd) && ImGui::BeginPopupModal(Translate(TR::OPERATION_SUCCESS), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        {
            if (popupTimerActive)
            {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - popupOpenTime).count();
                
                if (elapsed >= 300) // 5 minutes = 300 secondes
                {
                    ImGui::CloseCurrentPopup();
                    popupTimerActive = false;
                }
            }
            
            ImGui::Text(statusMessage.c_str());
            ImGui::Separator();
            ImGui::Spacing();
            if (ImGui::Button((std::string(Translate(TR::OK)) + "###Popup_Success_OK_Button").c_str(), ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                popupTimerActive = false;
            }
            ImGui::EndPopup();
        }
        else
        {
            if (popupTimerActive && !ImGui::IsPopupOpen(ImGuiID(0), ImGuiPopupFlags_AnyPopupId))
            {
                popupTimerActive = false;
            }
        }

        ImGui::Render();
        glViewport(0, 0, g_Width, g_Height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (needsSave)
        {
            SaveConfiguration(selectedFolders, destinationFolder);
            needsSave = false;
        }

        ::SwapBuffers(g_MainWindow.hDC);
    }

    SaveConfiguration(selectedFolders, destinationFolder);

    RemoveTrayIcon(hwnd);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CoUninitialize();

    CleanupDeviceWGL(hwnd, &g_MainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    GdiplusShutdown(g_gdiplusToken);

    return 0;
}
