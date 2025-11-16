#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include <GL/gl.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_ICON 1
#define ID_TRAY_SHOW 2
#define ID_TRAY_EXIT 3

struct WGL_WindowData { 
    HDC hDC; 
};

bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HICON LoadIconFromFile(const wchar_t* filePath, int width, int height);
HICON LoadIconFromMemory(const unsigned char* data, unsigned int dataSize, int width, int height);
bool CreateTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
void ShowTrayContextMenu(HWND hWnd);
void ShowTrayNotification(HWND hWnd, const wchar_t* title, const wchar_t* message);

extern HGLRC g_hRC;
extern WGL_WindowData g_MainWindow;
extern int g_Width;
extern int g_Height;
extern bool g_IsMinimizedToTray;

#endif
