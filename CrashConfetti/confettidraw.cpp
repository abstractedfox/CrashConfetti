#pragma once
#include <windows.h>
#include <iostream>
#include "CPlayer.h"
#include <string>
#include <pathcch.h>
//#include <gdiplusheaders.h>
//#include <gdiplusgraphics.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void drawTest(HWND& hwnd);
void openVideo(HWND hwnd);
void UpdateUI(HWND hwnd, PlayerState state);
void OnPaint(HWND hwnd);
void paintWindowTransparent(HWND hwnd);
void OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr);
LRESULT OnCreateWindow(HWND hwnd);
int Initialize(HINSTANCE hInstance, int nCmdShow);
void windowToTop(HWND hwnd);

HINSTANCE g_hInstance; //from cplayer example "current instance"
BOOL g_bRepaintClient = TRUE; //from cplayer example; "repaint the client application area?"
CPlayer* g_pPlayer = NULL;

int globalH, globalW;

//const std::wstring videoPath = L"C:\\Users\\coldc\\Documents\\miscfiles\\rat.mp4";
const std::wstring videoFileName = L"rat.mp4";
std::wstring videoPath = L"";
bool videoHasStarted;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	//Initialize globals
	videoHasStarted = false;
	globalH = 0;
	globalW = 0;

	//Look in the folder containing the exe for a video to use
	wchar_t localpath[1000]; //if the path to this is longer than 1000 characters you've got more issues than windows explorer crashing
	for (int i = 0; i < 1000; i++) localpath[i] = L'\0';
	GetModuleFileNameW(NULL, localpath, 1000);
	int slashPos = 0;
	for (int i = 0; i < sizeof(localpath); i++) {
		if (localpath[i] != L'\0') videoPath += &localpath[i];
		else break;

		if (localpath[i] == L'\\') slashPos = i;
	}
	videoPath = videoPath.substr(0, slashPos + 1);
	videoPath += videoFileName;

	Initialize(hInstance, nCmdShow);

	MSG message = {};
	//GetMessage always returns > 0 unless the program is exiting, breaking the loop
	while (GetMessage(&message, NULL, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return 573;
}

int Initialize(HINSTANCE hInstance, int nCmdShow) {
	const wchar_t CLASS_NAME[] = L"It's time for mf confetti";
	const bool overscan = true;
	const int overscanAmnt = 12;

	WNDCLASS windowclass = { };

	windowclass.lpfnWndProc = WindowProc;
	windowclass.hInstance = hInstance;
	windowclass.lpszClassName = CLASS_NAME;

	RegisterClass(&windowclass);

	/*
	MONITORINFOEXW monitor;
	monitor.cbSize = sizeof(MONITORINFOEXW);
	bool monitorWorked = GetMonitorInfoW(MonitorFromWindow(GetActiveWindow(), MONITOR_DEFAULTTONEAREST), &monitor);
	
	std::wstring rectout = std::to_wstring(monitor.rcMonitor.left) + L' ' + std::to_wstring(monitor.rcMonitor.top)
		+ L' ' + std::to_wstring(monitor.rcMonitor.right) + L' ' + std::to_wstring(monitor.rcMonitor.bottom);
		*/

	HDC hdc = GetDC(NULL);
	//int screenW = GetDeviceCaps(hdc, HORZRES), screenH = GetDeviceCaps(hdc, VERTRES);
	int screenW = GetSystemMetrics(SM_CXFULLSCREEN), screenH = GetSystemMetrics(SM_CYFULLSCREEN);
	int titleH = GetSystemMetrics(SM_CYSIZE);

	globalH = screenH + titleH;
	globalW = screenW;

	if (overscan) {
		globalH += 150 + overscanAmnt * 2; //yes it takes this much to get over the taskbar
		globalW += overscanAmnt * 2; //extra since the H value undercounts due to the task bar
	}

	int windowPosW = 0, windowPosH = 0 - titleH;
	if (overscan) {
		windowPosW -= overscanAmnt;
		windowPosH -= overscanAmnt;
	}

	//MessageBox(NULL, res.c_str(), L"blah", MB_OK);

	HWND hwnd = CreateWindowEx(
		WS_EX_LAYERED, //"optional" window style; WS_EX_LAYERED is necessary for transparency
		CLASS_NAME,
		L"Confetti",
		WS_OVERLAPPEDWINDOW, //window style

		//wpos, hpos, wsize, hsize
		//CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
		//0, 0 - titleH, screenW, screenH + titleH,
		windowPosW, windowPosH, globalW, globalH,

		NULL, //parent window
		NULL, //menu
		hInstance,
		NULL
	);
	


	if (hwnd == NULL) return -1;

	

	g_hInstance = hInstance; //this is for the video playback

	COLORREF basecolor = 0x00000000; //last 3 bytes are bbggrr; first is always 00

	bool didTransparencyWork = SetLayeredWindowAttributes(hwnd, basecolor, 0, LWA_COLORKEY); //3rd arg is alpha

	//Window is hidden until playback begins
	ShowWindow(hwnd, SW_HIDE);

	//openVideo(hwnd);
	//UpdateWindow(hwnd);

	return 0;
}


//DispatchMessage causes the OS to call WindowProc
LRESULT	CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		//asdf
	case WM_CREATE:
		return OnCreateWindow(hwnd); //player initialized here

	case WM_DESTROY:
		PostQuitMessage(0); //causes the next GetMessage call to return 0
		return 0;

	case WM_PAINT:
		OnPaint(hwnd);
		break;

	case WM_APP_PLAYER_EVENT:
		//MessageBox(NULL, L"WM_APP_PLAYER_EVENT has been reached", L"main::WindowProc", MB_OK);
		OnPlayerEvent(hwnd, wParam);
		break;

	case WM_ERASEBKGND:
		// Suppress window erasing, to reduce flickering while the video is playing.
		return 1;

		return -1;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void paintWindowTransparent(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	HBRUSH brushy = (HBRUSH)COLOR_WINDOW + 1;
	brushy = CreateSolidBrush(RGB(0, 0, 0xff));
	FillRect(hdc, &ps.rcPaint, brushy);
	//drawTest(hwnd);

	EndPaint(hwnd, &ps);
}

void drawImage(HDC hdc) {
	std::wstring testdir = L"C:\\Users\\coldc\\Documents\\miscfiles", filename = L"kitty.jpg";

	SetCurrentDirectoryW(testdir.c_str());
	//Image image = Image(filename.c_str());
	//Image* imageptr = &image;
	//Graphics graphics = Graphics::FromImage(&image);
	//Graphics graphics = Graphics(Graphics(&hdc));
	//Graphics graphica(hdc);


	//Graphics graphics = Graphics(&hwnd);

	//graphics.DrawImage(&image, 50, 50);
}

void drawTest(HWND& hwnd) {
	HDC hdc;
	hdc = GetDC(hwnd);
	//SelectObject(hdc, )


	MoveToEx(hdc, 10, 10, (LPPOINT)NULL);
	LineTo(hdc, 15, 30);

	COLORREF fontcolor = 0x000000ff, alpha = 0x00ff0000;

	SetTextColor(hdc, fontcolor);
	SetBkColor(hdc, alpha);


	RECT textrect;
	textrect.left = 30;
	textrect.right = 600;
	textrect.top = 60;
	textrect.bottom = 900;
	std::wstring texty = L"Goodbye Bitch!!";
	MoveToEx(hdc, 50, 60, (LPPOINT)NULL);
	DrawText(hdc, texty.c_str(), texty.length(), &textrect, DT_CENTER);
}

//this is necessary if video playback is paused & the window is redrawn, should be called by the system
void OnPaint(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	if (g_pPlayer && g_pPlayer->HasVideo()) {
		//MessageBox(NULL, L"There is video!!!!", L"main::OnPaint", MB_OK);

		g_pPlayer->Repaint();
	}
	else {
		//MessageBox(NULL, L"There is no video!!!!", L"main::OnPaint", MB_OK);
		//RECT rc;
		//GetClientRect(hwnd, &rc);
		//FillRect(hdc, &rc, (HBRUSH)COLOR_WINDOW);
	}
	EndPaint(hwnd, &ps);
}

void openVideo(HWND hwnd) {
	const bool debug = false;
	HRESULT hr;
	/* asdf
	hr = CPlayer::CreateInstance(hwnd, hwnd, &g_pPlayer); //initialize the player
	if (FAILED(hr)) {
		MessageBox(NULL, L"CPlayer::CreateInstance failed!!!", L"main::openVideo", MB_OK);
	}
	if (SUCCEEDED(hr)) {
		MessageBox(NULL, L"CPlayer::CreateInstance worked!!!", L"main::openVideo", MB_OK);
		//UpdateUI(hwnd, Closed);
	}*/

	//hr = g_pPlayer->ResizeVideo((WORD)50, (WORD)50);

	hr = g_pPlayer->OpenURL(videoPath.c_str());




	if (FAILED(hr)) {
		MessageBox(NULL, L"g_pPlayer->OpenURL failed!!!", L"main::openVideo", MB_OK);
	}
	if (SUCCEEDED(hr)) {
		if (debug) {
			MessageBox(NULL, L"g_pPlayer->OpenURL HUGE SUCCESS!!!", L"main::openVideo", MB_OK);
			if (g_pPlayer->HasVideo()) {
				MessageBox(NULL, L"g_pPlayer->HasVideo == true", L"main::openVideo", MB_OK);
			}
			else {
				MessageBox(NULL, L"g_pPlayer->HasVideo == false", L"main::openVideo", MB_OK);
			}
		}
		UpdateUI(hwnd, OpenPending);

		//MSG message = { hwnd, WM_APP_PLAYER_EVENT, 0, 0, GetMessageTime(), {0,0} };
		//DispatchMessage(&message);

	}

}

void UpdateUI(HWND hwnd, PlayerState state) {
	BOOL bWaiting = FALSE;
	BOOL bPlayback = FALSE;

	assert(g_pPlayer != NULL);

	switch (state)
	{
	case OpenPending:
		bWaiting = TRUE;
		break;

	case Started:
		bPlayback = TRUE;
		break;

	case Paused:
		bPlayback = TRUE;
		break;
	}

	if (bPlayback && g_pPlayer->HasVideo()) g_bRepaintClient = FALSE;
	else g_bRepaintClient = TRUE;
}

void OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr)
{
	//this function actually initiates playback


	HRESULT hr = g_pPlayer->HandleEvent(pUnkPtr);
	ShowWindow(hwnd, SW_NORMAL);
	windowToTop(hwnd);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"g_pPlayer->HandleEvent The badness!!!", L"main::OnPlayerEvent", MB_OK);
	}
	UpdateUI(hwnd, g_pPlayer->GetState());

	if (g_pPlayer->GetState() == Started) {
		videoHasStarted = true;
	}
	if (g_pPlayer->GetState() == Stopped && videoHasStarted) {
		ExitProcess(0);
	}

}


//asdf
LRESULT OnCreateWindow(HWND hwnd) {
	HRESULT hr;
	hr = CPlayer::CreateInstance(hwnd, hwnd, &g_pPlayer, globalW, globalH); //initialize the player
	if (SUCCEEDED(hr)) {
		//MessageBox(NULL, L"CPlayer::CreateInstance worked!!!", L"main::OnCreateWindow", MB_OK);
		//UpdateUI(hwnd, Closed);
		openVideo(hwnd);
		

		return 0;
	}
	else {
		//MessageBox(NULL, L"CPlayer::CreateInstance failed!!!", L"main::OnCreateWindow", MB_OK);
		//return -1;
	}
}

void windowToTop(HWND hwnd) {
	RECT currentWindow;
	GetWindowRect(hwnd, &currentWindow);
	SetWindowPos(hwnd, HWND_TOPMOST, currentWindow.left, currentWindow.top, currentWindow.right, currentWindow.bottom, SWP_NOSIZE);

}

/*
int main() {
	std::wcout << L"It's time to mf party";
	return 0;
}
*/