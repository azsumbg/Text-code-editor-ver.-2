#include "framework.h"
#include "Text code editor ver. 2.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "EditSrv.h"
#include <chrono>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "editsrv.lib")

const wchar_t bWinClassName[]{ L"EditD2" };
const char tmp_file[]{ ".\\res\\data\\temp.dat" };
const wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
const char help_file[]{ ".\\res\\data\\help.dat" };

const int mNew{ 1001 };
const int mExit{ 1002 };
const int mSave{ 1003 };
const int mLoad{ 1004 };

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HWND editHwnd{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HICON bIcon{ nullptr };
HCURSOR bCursor{ nullptr };
HCURSOR outCursor{ nullptr };
MSG bMsg{};
BOOL bRet{ 0 };

POINT cur_pos{};

D2D1_RECT_F b1Rect{ 20.0f, 5.0f, scr_width / 3.0f - 20.0f, 45.0f };
D2D1_RECT_F b2Rect{ scr_width / 3.0f + 20.0f, 5.0f, scr_width * 2.0f / 3.0f - 20.0f, 45.0f };
D2D1_RECT_F b3Rect{ scr_width * 2.0f / 3.0f + 20.0f, 5.0f, scr_width - 20.0f, 45.0f };

D2D1_RECT_F b1TxtRect{ 30.0f, 5.0f, scr_width / 3.0f - 20.0f, 45.0f };
D2D1_RECT_F b2TxtRect{ scr_width / 3.0f + 30.0f, 5.0f, scr_width * 2.0f / 3.0f - 20.0f, 45.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2.0f / 3.0f + 30.0f, 5.0f, scr_width - 20.0f, 45.0f };

float x_scale{ 0 };
float y_scale{ 0 };

wchar_t file_in_use[100]{ L".\\res\\data\\text.dat" };














int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    

    return (int) msg.wParam;
}