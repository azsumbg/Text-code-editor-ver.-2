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
HICON saveIcon{ nullptr };
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

int client_ex{ (int)(scr_width) };
int client_ey{ (int)(scr_height - 50.0f) };

wchar_t file_in_use[50]{ L".\\res\\data\\text.dat" };
wchar_t file_name[20]{L"\0"};

bool pause = false;
bool in_client = true;
bool show_help = false;

bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

edit::RANDIT RandIt{};

edit::BAG<wchar_t> TxtBag(50);

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
int GetFrame()
{
	static int frame{ 0 };
	static int max_frames{ 29 };
	static int frame_delay{ 2 };

	--frame_delay;
	if (frame_delay <= 0)
	{
		frame_delay = 2;

		++frame;

		if (frame > max_frames)frame = 0;
	}

	return frame;
}

void InitEditor()
{
	wcscpy_s(file_in_use, L".\\res\\data\\text.dat");
	TxtBag.clear();
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(saveIcon));
		return true;

	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;

		case IDOK:
			if (GetDlgItemText(hwnd, IDC_NAME, file_name, 20) < 1)
			{
				EndDialog(hwnd, IDCANCEL);
				break;
			}
			wcscpy_s(file_in_use, L".\\res\\data\\");
			wcscat_s(file_in_use, file_name);
			wcscat_s(file_in_use, L".dat");
			break;
		}
		break;
	}
	
	return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_CREATE:
		bBar = CreateMenu();
		bMain = CreateMenu();
		bStore = CreateMenu();

		AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
		AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");
		
		AppendMenu(bMain, MF_STRING, mNew, L"Нов документ");
		AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
		AppendMenu(bMain, MF_STRING, mExit, L"Изход");

		AppendMenu(bStore, MF_STRING, mSave, L"Запази документ");
		AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
		AppendMenu(bStore, MF_STRING, mLoad, L"Зареди документ");
		SetMenu(hwnd, bBar);

		editHwnd = CreateWindow(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | 
			ES_MULTILINE, 0, 50, client_ex, client_ey, hwnd, NULL, bIns, NULL);

		if (!editHwnd)
		{
			LogErr(L"Error creating edit control in main window !");
			ErrExit(eClass);
		}

		InitEditor();
		break;

	case WM_CLOSE:
		pause = true;
		if (!TxtBag.empty())
		{
			mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
			if (MessageBox(hwnd, L"Има текст, който не е запазен и ще го загубиш !\n\nНаистина ли излизаш ?",
				L"Изход ?", MB_YESNO | MB_ICONASTERISK | MB_APPLMODAL) == IDNO)
			{
				pause = false;
				break;
			}
		}
		else
		{
			mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
			if (MessageBox(hwnd, L"Наистина ли излизаш ?",
				L"Изход ?", MB_YESNO | MB_ICONASTERISK | MB_APPLMODAL) == IDNO)
			{
				pause = false;
				break;
			}
		}
		DestroyWindow(hwnd);
		break;










	case WM_DESTROY:
		ClearResources();
		PostQuitMessage(0);
		break;


	default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
	}


	return (LRESULT)(FALSE);
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