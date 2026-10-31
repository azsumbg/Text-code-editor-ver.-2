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

const int EDIT_BUTTON{ 3003 };

const int mNew{ 1001 };
const int mExit{ 1002 };
const int mSave{ 1003 };
const int mLoad{ 1004 };

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HWND editHwnd{ nullptr };
HWND statusHwnd{ nullptr };
HWND logoHwnd{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HICON bIcon{ nullptr };
HCURSOR bCursor{ nullptr };
HCURSOR outCursor{ nullptr };
HICON saveIcon{ nullptr };
MSG bMsg{};
BOOL bRet{ 0 };
UINT bTimer{ 0 };

POINT cur_pos{};

D2D1_RECT_F b1Rect{ 20.0f, 5.0f, scr_width / 3.0f - 20.0f, 45.0f };
D2D1_RECT_F b2Rect{ scr_width / 3.0f + 20.0f, 5.0f, scr_width * 2.0f / 3.0f - 20.0f, 45.0f };
D2D1_RECT_F b3Rect{ scr_width * 2.0f / 3.0f + 20.0f, 5.0f, scr_width - 20.0f, 45.0f };

D2D1_RECT_F b1TxtRect{ 30.0f, 10.0f, scr_width / 3.0f - 20.0f, 45.0f };
D2D1_RECT_F b2TxtRect{ scr_width / 3.0f + 30.0f, 10.0f, scr_width * 2.0f / 3.0f - 20.0f, 45.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2.0f / 3.0f + 50.0f, 10.0f, scr_width - 20.0f, 45.0f };

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

int current_text_code{ 0 };

edit::RANDIT RandIt{};

edit::BAG<wchar_t> TxtBag(50);

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };
ID2D1HwndRenderTarget* LogoDraw{ nullptr };

ID2D1SolidColorBrush* statBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmFormat{ nullptr };

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
	if (!FreeMem(&LogoDraw))LogErr(L"Error unloading D2D1HwndRenderTarget !");
	if (!FreeMem(&statBrush))LogErr(L"Error unloading D2D1 statBrush !");
	if (!FreeMem(&txtBrush))LogErr(L"Error unloading D2D1 txtBrush !");
	if (!FreeMem(&hgltBrush))LogErr(L"Error unloading D2D1 hgltBrush !");
	if (!FreeMem(&b1BckgBrush))LogErr(L"Error unloading D2D1 b1BckgBrush !");
	if (!FreeMem(&b2BckgBrush))LogErr(L"Error unloading D2D1 b2BckgBrush !");
	if (!FreeMem(&b3BckgBrush))LogErr(L"Error unloading D2D1 b3BckgBrush !");
	if (!FreeMem(&iWriteFactory))LogErr(L"Error unloading D2D1 iWriteFactory !");
	if (!FreeMem(&nrmFormat))LogErr(L"Error unloading D2D1 nrmFormat !");
	
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

	current_text_code = RandIt(50, 60000);
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
		SetTimer(hwnd, (UINT_PTR)(bTimer), 30000, NULL);
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

		statusHwnd = CreateWindow(L"EDIT", NULL, WS_CHILD | WS_VISIBLE, 0, 0, (int)(scr_width), 50, hwnd, NULL, bIns, NULL);
		
		logoHwnd = CreateWindow(L"EDIT", NULL, WS_CHILD | WS_VISIBLE, 0, 0, (int)(scr_width), int(scr_height), hwnd, NULL, bIns, 
			NULL);

		if (!editHwnd || !statusHwnd || !logoHwnd)
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

	case WM_SETCURSOR:
		GetCursorPos(&cur_pos);
		ScreenToClient(hwnd, &cur_pos);
		if (LOWORD(lParam) == HTCLIENT)
		{
			in_client = true;
			pause = false;

			ShowWindow(logoHwnd, false);

			if (cur_pos.y * y_scale < 50)
			{
				if (cur_pos.x * x_scale >= b1Rect.left && cur_pos.x * x_scale <= b1Rect.right)
				{
					if (!b1Hglt)
					{
						mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = true;
						b2Hglt = false;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * x_scale >= b2Rect.left && cur_pos.x * x_scale <= b2Rect.right)
				{
					if (!b2Hglt)
					{
						mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = true;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * x_scale >= b3Rect.left && cur_pos.x * x_scale <= b3Rect.right)
				{
					if (!b3Hglt)
					{
						mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = false;
						b3Hglt = true;
					}
				}
				else if (b1Hglt || b2Hglt || b3Hglt)
				{
					mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
					b1Hglt = false;
					b2Hglt = false;
					b3Hglt = false;
				}

				SetCursor(outCursor);

				return true;
			}
			else if (b1Hglt || b2Hglt || b3Hglt)
			{
				mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
				b1Hglt = false;
				b2Hglt = false;
				b3Hglt = false;
			}
			
			SetCursor(bCursor);

			return true;
		}
		else
		{
			in_client = false;
			pause = true;

			ShowWindow(logoHwnd, true);

			if (b1Hglt || b2Hglt || b3Hglt)
			{
				mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
				b1Hglt = false;
				b2Hglt = false;
				b3Hglt = false;
			}

			SetCursor(LoadCursor(NULL, IDC_ARROW));

			return true;
		}
		break;

	case WM_TIMER:
		if (pause)break;
		else
		{
			int buffer_size{ GetWindowTextLengthW(editHwnd) + 1 };
			
			if (buffer_size > 1)
			{
				wchar_t* txt_buffer{ reinterpret_cast<wchar_t*>(calloc(buffer_size, sizeof(wchar_t))) };

				if (!txt_buffer)throw edit::EXCEPTION(BAD_PTR);
				else
				{
					GetWindowTextW(editHwnd, txt_buffer, buffer_size);

					TxtBag.clear();
					for (int i = 0; i < buffer_size; ++i)TxtBag.push_back(txt_buffer[i]);
				}

				free(txt_buffer);
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case mNew:
			if (!TxtBag.empty())
			{
				mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
				if (MessageBox(hwnd, L"Има текст, който не е запазен и ще го загубиш !\n\nНаистина ли изчистваш всичко ?",
					L"Нов документ ?", MB_YESNO | MB_ICONASTERISK | MB_APPLMODAL) == IDNO)
				{
					pause = false;
					break;
				}
			}
			InitEditor();
			break;

		case mExit:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			break;

		case mSave:
			if (TxtBag.empty())
			{
				int buffer_size{ GetWindowTextLengthW(editHwnd) + 1 };

				if (buffer_size > 1)
				{
					wchar_t* txt_buffer{ reinterpret_cast<wchar_t*>(calloc(buffer_size, sizeof(wchar_t))) };

					if (!txt_buffer)throw edit::EXCEPTION(BAD_PTR);
					else
					{
						GetWindowTextW(editHwnd, txt_buffer, buffer_size);

						TxtBag.clear();
						for (int i = 0; i < buffer_size; ++i)TxtBag.push_back(txt_buffer[i]);
					}

					free(txt_buffer);
				}
				mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
				MessageBox(hwnd, L"Липсва текст, който да бъде запазен !\n\nПостарай се повече !",
					L"Няма текст !", MB_OK | MB_APPLMODAL | MB_ICONERROR);
				break;
			}
			else edit::SaveFile(file_in_use, TxtBag, current_text_code, false);
			break;
		}
		break;

	case WM_SIZE:
		client_ex = LOWORD(lParam);
		client_ey = HIWORD(lParam);
		MoveWindow(editHwnd, 0, 50, client_ex, client_ey, true);
		MoveWindow(statusHwnd, 0, 0, client_ex, 50, true);
		break;

	case WM_DESTROY:
		ClearResources();
		PostQuitMessage(0);
		break;


	default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
	}


	return (LRESULT)(FALSE);
}

void CreateResources()
{
	int win_x{ GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2.0f) };
	int win_y = 50;

	if (win_x + (int)(scr_width) > GetSystemMetrics(SM_CXSCREEN) || win_y + (int)(scr_height) > GetSystemMetrics(SM_CYSCREEN))
		ErrExit(eScreen);
	
	bIcon = (HICON)(LoadImage(NULL, L".\\res\\bicon.ico", IMAGE_ICON, 64, 64, LR_LOADFROMFILE));
	saveIcon = (HICON)(LoadImage(NULL, L".\\res\\save.ico", IMAGE_ICON, 64, 64, LR_LOADFROMFILE));

	if (!bIcon || !saveIcon)ErrExit(eIcon);

	bCursor = LoadCursorFromFile(L".\\res\\bcursor.ani");
	outCursor = LoadCursorFromFile(L".\\res\\out.ani");

	if (!bIcon || !outCursor)ErrExit(eCursor);

	bWinClass.lpszClassName = bWinClassName;
	bWinClass.hInstance = bIns;
	bWinClass.lpfnWndProc = &WinProc;
	bWinClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 100));
	bWinClass.hIcon = bIcon;
	bWinClass.hCursor = bCursor;
	bWinClass.style = CS_DROPSHADOW;

	if (!RegisterClass(&bWinClass))ErrExit(eClass);

	bHwnd = CreateWindow(bWinClassName, L"TEXT EDITOR 2.0", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, win_x, win_y,
		(int)(scr_width), (int)(scr_height), NULL, NULL, bIns, NULL);
	if (!bHwnd)ErrExit(eWindow);
	else
	{
		ShowWindow(bHwnd, SW_SHOWDEFAULT);

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
		if (hr != S_OK)
		{
			LogErr(L"Error creating main D2D1 Factory !");
			ErrExit(eD2D);
		}

		if (iFactory && statusHwnd)
		{
			hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(statusHwnd,
				D2D1::SizeU((UINT32)(scr_width), 50)), &Draw);
			if (hr != S_OK)
			{
				LogErr(L"Error creating main D2D1 HwndRenderTarget !");
				ErrExit(eD2D);
			}
		}

		if (Draw && statusHwnd)
		{
			RECT DPIRect{};
			D2D1_SIZE_F DIPSize{ Draw->GetSize() };

			GetClientRect(statusHwnd, &DPIRect);

			x_scale = DIPSize.width / (DPIRect.right - DPIRect.left);
			y_scale = DIPSize.height / (DPIRect.bottom - DPIRect.top);

			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &statBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cyan), &txtBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Chartreuse), &hgltBrush);

			D2D1_GRADIENT_STOP gStops[2]{};
			ID2D1GradientStopCollection* gColl{ nullptr };

			gStops[0].position = 0;
			gStops[0].color = D2D1::ColorF(D2D1::ColorF::DarkSlateBlue);
			gStops[1].position = 1.0f;
			gStops[1].color = D2D1::ColorF(D2D1::ColorF::MidnightBlue);

			hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 GradientStopCollection for buttons background !");
				ErrExit(eD2D);
			}

			if (gColl)
			{
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left +
					(b1Rect.right - b1Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0), (b1Rect.right - b1Rect.left) / 2.0f, 25.0f),
					gColl, &b1BckgBrush);
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left +
					(b2Rect.right - b2Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0), (b2Rect.right - b2Rect.left) / 2.0f, 25.0f),
					gColl, &b2BckgBrush);
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left +
					(b3Rect.right - b3Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0), (b3Rect.right - b3Rect.left) / 2.0f, 25.0f),
					gColl, &b3BckgBrush);

				if (hr != S_OK)
				{
					LogErr(L"Error creating D2D1 RadialGradientBrushes for buttons background !");
					ErrExit(eD2D);
				}

				if (!FreeMem(&gColl))
				{
					LogErr(L"Error releasing GradientStopCollection for button backgrounds !");
					ErrExit(eD2D);
				}
			}
		}

		if (iFactory && logoHwnd)
		{
			hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(logoHwnd,
				D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &LogoDraw);
			if (hr != S_OK)
			{
				LogErr(L"Error creating main logo D2D1 HwndRenderTarget !");
				ErrExit(eD2D);
			}
		}

		if (LogoDraw)
		{
			bmpLogo = Load(L".\\res\\img\\logo.png", LogoDraw);
			if (!bmpLogo)
			{
				LogErr(L"Error loading bmpLogo !");
				ErrExit(eD2D);
			}

			for (int i = 0; i < 30; ++i)
			{
				wchar_t name[50]{ L".\\res\\img\\intro\\" };
				wchar_t add[5]{};

				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");

				bmpIntro[i] = Load(name, LogoDraw);
				if (!bmpIntro[i])
				{
					LogErr(L"Error loading bmpIntro set !");
					ErrExit(eD2D);
				}
			}
			for (int i = 0; i < 30; ++i)
			{
				wchar_t name[50]{ L".\\res\\img\\pause\\" };
				wchar_t add[5]{};

				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");

				bmpPause[i] = Load(name, LogoDraw);
				if (!bmpPause[i])
				{
					LogErr(L"Error loading bmpPause set !");
					ErrExit(eD2D);
				}
			}
		}

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>
			(&iWriteFactory));
		if (hr != S_OK)
		{
			LogErr(L"Error creating main D2D1 WriteFactory !");
			ErrExit(eD2D);
		}

		if (iWriteFactory)
		{
			hr = iWriteFactory->CreateTextFormat(L"GNABRI", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
				DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"", &nrmFormat);
			if (hr != S_OK)
			{
				LogErr(L"Error creating main D2D1 WriteFactory text format !");
				ErrExit(eD2D);
			}
		}
	}

	PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_ASYNC);

	for (int i = 0; i < 500; ++i)
	{
		LogoDraw->BeginDraw();
		LogoDraw->Clear();
		LogoDraw->DrawBitmap(bmpIntro[GetFrame()], FULL_SCREEN);
		LogoDraw->DrawBitmap(bmpLogo, FULL_SCREEN);
		LogoDraw->EndDraw();
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	bIns = hInstance;
	if (!bIns)
	{
		LogErr(L"Error obtaining hInstance from Windows !");
		ErrExit(eClass);
	}

	CreateResources();

	while (bMsg.message != WM_QUIT)
	{
		if ((bRet = PeekMessage(&bMsg, NULL, NULL, NULL, PM_REMOVE)) != 0)
		{
			if (bRet == -1)ErrExit(eMsg);

			TranslateMessage(&bMsg);
			DispatchMessage(&bMsg);
		}

		if (pause)
		{
			if (show_help)continue;
			LogoDraw->BeginDraw();
			LogoDraw->DrawBitmap(bmpPause[GetFrame()], FULL_SCREEN);
			LogoDraw->EndDraw();
			continue;
		}

		Draw->BeginDraw();

		if (statBrush && txtBrush && hgltBrush && b1BckgBrush && b2BckgBrush && b3BckgBrush && nrmFormat)
		{
			Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statBrush);
		
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 20.0f, 25.0f), b1BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 20.0f, 25.0f), b2BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 20.0f, 25.0f), b3BckgBrush);

			if (!b1Hglt)Draw->DrawTextW(L"ЗАПАЗИ ТЕКСТ", 13, nrmFormat, b1TxtRect, txtBrush);
			else Draw->DrawTextW(L"ЗАПАЗИ ТЕКСТ", 13, nrmFormat, b1TxtRect, hgltBrush);
			if (!b2Hglt)Draw->DrawTextW(L"ЗАРЕДИ ТЕКСТ", 13, nrmFormat, b2TxtRect, txtBrush);
			else Draw->DrawTextW(L"ЗАРЕДИ ТЕКСТ", 13, nrmFormat, b2TxtRect, hgltBrush);
			if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ", 6, nrmFormat, b3TxtRect, txtBrush);
			else Draw->DrawTextW(L"ПОМОЩ", 6, nrmFormat, b3TxtRect, hgltBrush);
		}
		
		Draw->EndDraw();
	}
	
	ClearResources();
	std::remove(tmp_file);

	return (int) bMsg.wParam;
}