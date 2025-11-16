#include "window.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "preferences.h"
#include "file_utils.h"
#include "logo_embedded.h"
#include <shellapi.h>
#include <gdiplus.h>
#include <comdef.h>
#include <cstring>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

#ifndef NOTIFYICONDATAW_V3_SIZE
#define NOTIFYICONDATAW_V3_SIZE FIELD_OFFSET(NOTIFYICONDATAW, guid) + sizeof(GUID)
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HGLRC g_hRC = nullptr;
WGL_WindowData g_MainWindow;
int g_Width = 0;
int g_Height = 0;
bool g_IsMinimizedToTray = false;

bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
        g_hRC = wglCreateContext(data->hDC);
    return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    wglMakeCurrent(nullptr, nullptr);
    ::ReleaseDC(hWnd, data->hDC);
}

void ResetDeviceWGL()
{

}

extern ULONG_PTR g_gdiplusToken;

HICON LoadIconFromFile(const wchar_t* filePath, int width, int height)
{
    HICON hIcon = nullptr;
    
    Image* image = new Image(filePath);
    if (image && image->GetLastStatus() == Ok)
    {
        Bitmap* bitmap = new Bitmap(width, height, PixelFormat32bppARGB);
        if (bitmap)
        {
            Graphics graphics(bitmap);
            graphics.SetSmoothingMode(SmoothingModeHighQuality);
            graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
            graphics.DrawImage(image, 0, 0, width, height);
            
            bitmap->GetHICON(&hIcon);
            delete bitmap;
        }
        delete image;
    }
    
    return hIcon;
}

HICON LoadIconFromMemory(const unsigned char* data, unsigned int dataSize, int width, int height)
{
    HICON hIcon = nullptr;
    
    IStream* pStream = nullptr;
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dataSize);
    if (hGlobal)
    {
        void* pData = GlobalLock(hGlobal);
        if (pData)
        {
            memcpy(pData, data, dataSize);
            GlobalUnlock(hGlobal);
            
            if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK)
            {
                Image* image = new Image(pStream);
                if (image && image->GetLastStatus() == Ok)
                {
                    Bitmap* bitmap = new Bitmap(width, height, PixelFormat32bppARGB);
                    if (bitmap)
                    {
                        Graphics graphics(bitmap);
                        graphics.SetSmoothingMode(SmoothingModeHighQuality);
                        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
                        graphics.DrawImage(image, 0, 0, width, height);
                        
                        bitmap->GetHICON(&hIcon);
                        delete bitmap;
                    }
                    delete image;
                }
                pStream->Release();
            }
            else
            {
                GlobalFree(hGlobal);
            }
        }
        else
        {
            GlobalFree(hGlobal);
        }
    }
    
    return hIcon;
}

bool CreateTrayIcon(HWND hWnd)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = NOTIFYICONDATAW_V3_SIZE;
    nid.hWnd = hWnd;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    
    HICON hIcon = LoadIconFromMemory(g_logoData, g_logoDataSize, 16, 16);
    if (!hIcon)
    {
        hIcon = (HICON)LoadImageW(GetModuleHandleW(nullptr), 
            MAKEINTRESOURCEW(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        if (!hIcon)
        {
            hIcon = (HICON)LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 16, 16, LR_SHARED);
        }
    }
    nid.hIcon = hIcon;
    
    wcscpy_s(nid.szTip, L"BackupJZ");
    
    return Shell_NotifyIconW(NIM_ADD, &nid) != FALSE;
}

void RemoveTrayIcon(HWND hWnd)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = NOTIFYICONDATAW_V3_SIZE;
    nid.hWnd = hWnd;
    nid.uID = ID_TRAY_ICON;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

void ShowTrayContextMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);
    
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW, L"Afficher");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Quitter");
    
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
    DestroyMenu(hMenu);
}

void ShowTrayNotification(HWND hWnd, const wchar_t* title, const wchar_t* message)
{
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = NOTIFYICONDATAW_V3_SIZE;
    nid.hWnd = hWnd;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_INFO;
    
    if (title)
        wcscpy_s(nid.szInfoTitle, title);
    if (message)
        wcscpy_s(nid.szInfo, message);
    
    nid.uTimeout = 3000;
    
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            g_Width = LOWORD(lParam);
            g_Height = HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP)
        {
            ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
            g_IsMinimizedToTray = false;
        }
        else if (lParam == WM_RBUTTONUP)
        {
            ShowTrayContextMenu(hWnd);
        }
        return 0;
    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        g_IsMinimizedToTray = true;
        {
            Preferences prefs;
            LoadPreferences(prefs);
            if (prefs.showNotifications)
            {
                ShowTrayNotification(hWnd, L"BackupJZ - En arrière-plan", L"L'application continue de fonctionner en arrière-plan. Cliquez sur l'icône pour la restaurer.");
            }
        }
        return 0;
    case WM_COMMAND:
        if (wParam == ID_TRAY_SHOW)
        {
            ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
            g_IsMinimizedToTray = false;
        }
        else if (wParam == ID_TRAY_EXIT)
        {
            RemoveTrayIcon(hWnd);
            PostQuitMessage(0);
        }
        return 0;
    case WM_DESTROY:
        RemoveTrayIcon(hWnd);
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
