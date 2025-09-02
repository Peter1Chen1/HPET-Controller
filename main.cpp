#include <Windows.h>
#include <string>
#include <sstream>

#define ID_BTN_ENABLE   101
#define ID_BTN_DISABLE  102
#define ID_BTN_STATUS   103
#define ID_STATIC_STATUS 104

// 全局控件句柄
HWND hStatus = nullptr;
HWND hBtnEnable = nullptr;
HWND hBtnDisable = nullptr;
HWND hBtnStatus = nullptr;

// 查询 HPET 状态函数（返回 "已开启" 或 "已关闭"）
std::wstring GetHPETStatus() {
    FILE* pipe = _popen("bcdedit /enum | findstr useplatformclock", "r");
    if (!pipe) return L"无法获取状态";

    wchar_t buffer[128];
    std::wstring result;
    while (fgetws(buffer, 128, pipe)) {
        result += buffer;
    }
    _pclose(pipe);

    if (result.find(L"Yes") != std::wstring::npos)
        return L"HPET 已开启";
    else if (result.find(L"No") != std::wstring::npos)
        return L"HPET 已关闭";
    else
        return L"未知状态";
}

// 窗口过程
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            
            // 字体
            HFONT hFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");

            // 开启按钮
            hBtnEnable = CreateWindowW(L"BUTTON", L"开启 HPET",
                WS_VISIBLE | WS_CHILD,
                50, 50, 120, 40,
                hwnd, (HMENU)ID_BTN_ENABLE,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                nullptr);
            SendMessageW(hBtnEnable, WM_SETFONT, (WPARAM)hFont, TRUE);

            // 关闭按钮
            hBtnDisable = CreateWindowW(L"BUTTON", L"关闭 HPET",
                WS_VISIBLE | WS_CHILD,
                200, 50, 120, 40,
                hwnd, (HMENU)ID_BTN_DISABLE,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                nullptr);
            SendMessageW(hBtnDisable, WM_SETFONT, (WPARAM)hFont, TRUE);

            // 查看状态按钮
            hBtnStatus = CreateWindowW(L"BUTTON", L"查看状态",
                WS_VISIBLE | WS_CHILD,
                350, 50, 120, 40,
                hwnd, (HMENU)ID_BTN_STATUS,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                nullptr);
            SendMessageW(hBtnStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

            // 状态显示文本
            hStatus = CreateWindowW(L"STATIC", L"当前状态: 未操作",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                50, 120, 420, 30,
                hwnd, (HMENU)ID_STATIC_STATUS,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                nullptr);
            SendMessageW(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(0, 120, 215));       // 深蓝文字
            SetBkColor(hdcStatic, RGB(204, 204, 204));       // 浅灰背景
            static HBRUSH hBrush = CreateSolidBrush(RGB(204, 204, 204));
            return (INT_PTR)hBrush;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_ENABLE) {
                int ret = system("bcdedit /set useplatformclock yes");
                std::wstring msg = (ret == 0) ? L"HPET 已开启" : L"开启 HPET 失败";
                SetWindowTextW(hStatus, msg.c_str());
            }
            else if (LOWORD(wParam) == ID_BTN_DISABLE) {
                int ret = system("bcdedit /set useplatformclock no");
                std::wstring msg = (ret == 0) ? L"HPET 已关闭" : L"关闭 HPET 失败";
                SetWindowTextW(hStatus, msg.c_str());
            }
            else if (LOWORD(wParam) == ID_BTN_STATUS) {
                std::wstring status = L"当前状态: " + GetHPETStatus();
                SetWindowTextW(hStatus, status.c_str());
            }
            break;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSW myClass = { 0 };
    myClass.lpszClassName = L"HPETControllerWindow";
    myClass.lpfnWndProc = WindowProc;
    myClass.hIcon = (HICON)LoadImageW(NULL, L"HPET.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    myClass.hInstance = hInstance;
    myClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&myClass);

    HWND hwnd = CreateWindowW(
        myClass.lpszClassName,
        L"HPET 控制器",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 540, 250,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { 0 };
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}
