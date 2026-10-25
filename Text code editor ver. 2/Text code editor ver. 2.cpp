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

edit::RANDIT RandIt{};

edit::BAG<wchar_t> TxtBag(100);

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1SolidColorBrush* statBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmFormat{ nullptr };
IDWriteTextFormat* midFormat{ nullptr };
IDWriteTextFormat* bigFormat{ nullptr };

ID2D1Bitmap* bmpLogo{ nullptr };
ID2D1Bitmap* bmpIntro[30]{ nullptr };
ID2D1Bitmap* bmpPause[30]{ nullptr };

////////////////////////////////////////////////////

template<typename T>concept HasRelease = requires(T check)
{
	check.Release();
};
template<HasRelease T>bool FreeMem(T** var)
{
	if ((*var))
	{
		(*var)->Release();
		(*var) = nullptr;

		return true;
	}

	return false;
}
void LogErr(const wchar_t* what)
{
	std::wofstream err{ L".\\res\\data\\error.log",std::ios::app };
	err << what << L" time stamp: " << std::chrono::system_clock::now() << std::endl;
	err.close();
}
void ClearResources()
{
	if (!FreeMem(&iFactory))LogErr(L"Error unloading iFactory !");
	if (!FreeMem(&Draw))LogErr(L"Error unloading D2D1HwndRenderTarget !");
	if (!FreeMem(&statBrush))LogErr(L"Error unloading D2D1 statBrush !");
	if (!FreeMem(&txtBrush))LogErr(L"Error unloading D2D1 txtBrush !");
	if (!FreeMem(&hgltBrush))LogErr(L"Error unloading D2D1 hgltBrush !");
	if (!FreeMem(&inactBrush))LogErr(L"Error unloading D2D1 inactBrush !");
	if (!FreeMem(&b1BckgBrush))LogErr(L"Error unloading D2D1 b1BckgBrush !");
	if (!FreeMem(&b2BckgBrush))LogErr(L"Error unloading D2D1 b2BckgBrush !");
	if (!FreeMem(&b3BckgBrush))LogErr(L"Error unloading D2D1 b3BckgBrush !");
	if (!FreeMem(&iWriteFactory))LogErr(L"Error unloading D2D1 iWriteFactory !");
	if (!FreeMem(&nrmFormat))LogErr(L"Error unloading D2D1 nrmFormat !");
	if (!FreeMem(&midFormat))LogErr(L"Error unloading D2D1 midFormat !");
	if (!FreeMem(&bigFormat))LogErr(L"Error unloading D2D1 bigFormat !");

	if (!FreeMem(&bmpLogo))LogErr(L"Error unloading D2D1 bmpLogo !");
	for (int i = 0; i < 29; ++i)if (!FreeMem(&bmpIntro[i]))LogErr(L"Error unloading D2D1 bmpIntro set !");
	for (int i = 0; i < 29; ++i)if (!FreeMem(&bmpPause[i]))LogErr(L"Error unloading D2D1 bmpPause set !");
}
void ErrExit(int what)
{
	MessageBeep(MB_ICONERROR);
	MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

	ClearResources();
	std::remove(tmp_file);
	exit(1);
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	bIns = hInstance;
	if (!bIns)
	{
		LogErr(L"Error obtaining hInstance from Windows !");
		ErrExit(eClass);
	}



    
	
	





	
	ClearResources();
	std::remove(tmp_file);

	return (int) bMsg.wParam;
}