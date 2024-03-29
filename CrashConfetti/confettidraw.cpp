/*
confettidraw.cpp, copyright Chris/abstractedfox 2022-2023. Plays an mp4 file covering most of a monitor, utilizing transparency, and exits.

This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

In addition to the standard GPL v3.0, this program includes an additional condition:
Redistribution or modification of this code may not strike credit of the original author, Chris/abstractedfox

Contact: chriswhoprograms@gmail.com
*/

#pragma once
#include <windows.h>
#include <iostream>
#include "CPlayer.h"
#include <string>
#include <pathcch.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void drawTest(HWND& hwnd);
void openVideo(HWND hwnd);
void OnPaint(HWND hwnd);
void OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr);
LRESULT OnCreateWindow(HWND hwnd);
int Initialize(HINSTANCE hInstance, int nCmdShow);
void windowToTop(HWND hwnd);

HINSTANCE g_hInstance; //from cplayer example "current instance"
//BOOL g_bRepaintClient = TRUE; //from cplayer example; "repaint the client application area."
CPlayer* g_pPlayer = NULL;

int globalH, globalW;


const std::wstring videoFileName = L"confetti.mp4";
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

	if (Initialize(hInstance, nCmdShow) != 0) return -1;

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

	g_hInstance = hInstance; //make this instance available to CPlayer

	COLORREF basecolor = 0x00000000; //last 3 bytes are bbggrr; first is always 00

	bool didTransparencyWork = SetLayeredWindowAttributes(hwnd, basecolor, 0, LWA_COLORKEY); //3rd arg is alpha

	//Window is hidden until playback begins
	ShowWindow(hwnd, SW_HIDE);

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
		OnPlayerEvent(hwnd, wParam);
		break;

	case WM_ERASEBKGND:
		// Suppress window erasing, to reduce flickering while the video is playing.
		return 1;

		return -1;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void drawTest(HWND& hwnd) {
	HDC hdc;
	hdc = GetDC(hwnd);


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
	std::wstring texty = L"Goodbye!!";
	MoveToEx(hdc, 50, 60, (LPPOINT)NULL);
	DrawText(hdc, texty.c_str(), texty.length(), &textrect, DT_CENTER);
}

//this is necessary if video playback is paused & the window is redrawn, should be called by the system
void OnPaint(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	if (g_pPlayer && g_pPlayer->HasVideo()) {
		g_pPlayer->Repaint();
	}
	EndPaint(hwnd, &ps);
}


void openVideo(HWND hwnd) {
	const bool debug = false;
	HRESULT hr;

	hr = g_pPlayer->OpenURL(videoPath.c_str());

	if (FAILED(hr)) {
		if (debug) MessageBox(NULL, L"g_pPlayer->OpenURL failed!!!", L"main::openVideo", MB_OK);
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
	}

}

void OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr)
{
	const bool debug = false;
	//this function actually initiates playback

	HRESULT hr = g_pPlayer->HandleEvent(pUnkPtr); //Playback starts
	ShowWindow(hwnd, SW_NORMAL);
	windowToTop(hwnd);
	if (FAILED(hr))
	{
		if (debug) MessageBox(NULL, L"g_pPlayer->HandleEvent The badness!!!", L"main::OnPlayerEvent", MB_OK);
	}

	if (g_pPlayer->GetState() == Started) {
		videoHasStarted = true;
	}
	if (g_pPlayer->GetState() == Stopped && videoHasStarted) {
		ExitProcess(0);
	}

}


LRESULT OnCreateWindow(HWND hwnd) {
	const bool debug = false;
	HRESULT hr;
	hr = CPlayer::CreateInstance(hwnd, hwnd, &g_pPlayer, globalW, globalH); //initialize the player
	if (SUCCEEDED(hr)) {
		if (debug) MessageBox(NULL, L"CPlayer::CreateInstance worked!!!", L"main::OnCreateWindow", MB_OK);
		openVideo(hwnd); //The WM_APP_PLAYER_EVENT event that initiates playback is posted

		return 0;
	}
	else {
		if (debug) MessageBox(NULL, L"CPlayer::CreateInstance failed!!!", L"main::OnCreateWindow", MB_OK);
		return -1;
	}
}

void windowToTop(HWND hwnd) {
	RECT currentWindow;
	GetWindowRect(hwnd, &currentWindow);
	SetWindowPos(hwnd, HWND_TOPMOST, currentWindow.left, currentWindow.top, currentWindow.right, currentWindow.bottom, SWP_NOSIZE);

}
