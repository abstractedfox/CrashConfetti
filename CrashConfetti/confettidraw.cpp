#pragma once
#include <windows.h>
#include <iostream>
#include "CPlayer.h"
//#include <gdiplusheaders.h>
//#include <gdiplusgraphics.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void drawTest(HWND& hwnd);
void openVideo(HWND hwnd);
void UpdateUI(HWND hwnd, PlayerState state);
void OnPaint(HWND hwnd);
void paintWindowTransparent(HWND hwnd);
void OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr);

HINSTANCE g_hInstance; //from cplayer example "current instance"
BOOL g_bRepaintClient = TRUE; //from cplayer example; "repaint the client application area?"
CPlayer *g_pPlayer = NULL;

const std::wstring videoPath = L"C:\\Users\\coldc\\Documents\\miscfiles\\rat.mp4";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	const wchar_t CLASS_NAME[] = L"It's time for motherfucking confetti";
	
	WNDCLASS windowclass = { };

	windowclass.lpfnWndProc = WindowProc;
	windowclass.hInstance = hInstance;
	windowclass.lpszClassName = CLASS_NAME;
	
	RegisterClass(&windowclass);

	HWND hwnd = CreateWindowEx(
		WS_EX_LAYERED, //"optional" window style; WS_EX_LAYERED is necessary for transparency
		CLASS_NAME,
		L"Confetti",
		WS_OVERLAPPEDWINDOW, //window style

		//size/pos
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL, //parent window
		NULL, //menu
		hInstance,
		NULL
	);

	if (hwnd == NULL) return -1;
	
	g_hInstance = hInstance; //this is for the video playback

	COLORREF basecolor = 0x00FF0000; //last 3 bytes are bbggrr; first is always 00

	bool didTransparencyWork = SetLayeredWindowAttributes(hwnd, basecolor, 80, LWA_COLORKEY); //3rd arg is alpha


	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	openVideo(hwnd);

	MSG message = {};
	//GetMessage always returns > 0 unless the program is exiting, breaking the loop
	while (GetMessage(&message, NULL, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return 573;
}

//DispatchMessage causes the OS to call WindowProc
LRESULT	CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_DESTROY:
			PostQuitMessage(0); //causes the next GetMessage call to return 0
			return 0;

		case WM_PAINT:
			OnPaint(hwnd);
			break;

		case WM_APP_PLAYER_EVENT:
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

void drawTest(HWND &hwnd) {
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
		MessageBox(NULL, L"There is video!!!!", L"main::OnPaint", MB_OK);
		g_pPlayer->Repaint();
	}
	else {
		MessageBox(NULL, L"There is no video!!!!", L"main::OnPaint", MB_OK);
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect(hdc, &rc, (HBRUSH)COLOR_WINDOW);
	}
	EndPaint(hwnd, &ps);
}

void openVideo(HWND hwnd) {
	HRESULT hr;
	hr = CPlayer::CreateInstance(hwnd, hwnd, &g_pPlayer); //initialize the player
	if (FAILED(hr)) {
		MessageBox(NULL, L"CPlayer::CreateInstance failed!!!", L"main::openVideo", MB_OK);
	}
	if (SUCCEEDED(hr)) {
		UpdateUI(hwnd, Closed);
	}
	
	
	hr = g_pPlayer->OpenURL(videoPath.c_str());

	if (FAILED(hr)) {
		MessageBox(NULL, L"g_pPlayer->OpenURL failed!!!", L"main::openVideo", MB_OK);
	}
	if (SUCCEEDED(hr)) {
		MessageBox(NULL, L"g_pPlayer->OpenURL HUGE SUCCESS!!!", L"main::openVideo", MB_OK);
		UpdateUI(hwnd, OpenPending);

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
	HRESULT hr = g_pPlayer->HandleEvent(pUnkPtr);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"g_pPlayer->HandleEvent The badness!!!", L"main::OnPlayerEvent", MB_OK);
	}
	UpdateUI(hwnd, g_pPlayer->GetState());
}


/*
int main() {
	std::wcout << L"It's time to mf party";
	return 0;
}
*/