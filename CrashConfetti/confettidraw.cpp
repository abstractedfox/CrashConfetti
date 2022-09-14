
#include <windows.h>
#include <iostream>
#include <gdiplusheaders.h>
#include <gdiplusgraphics.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void drawTest(HWND& hwnd);

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
	
	COLORREF basecolor = 0x00FF0000; //last 3 bytes are bbggrr; first is always 00

	bool didTransparencyWork = SetLayeredWindowAttributes(hwnd, basecolor, 80, LWA_COLORKEY); //3rd arg is alpha

	ShowWindow(hwnd, nCmdShow);

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
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			HBRUSH brushy = (HBRUSH)COLOR_WINDOW + 1;
			brushy = CreateSolidBrush(RGB(0, 0, 0xff));
			FillRect(hdc, &ps.rcPaint, brushy);
			//drawTest(hwnd);

			EndPaint(hwnd, &ps);
		}

		return -1;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
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

/*
int main() {
	std::wcout << L"It's time to fuckin party";
	return 0;
}
*/