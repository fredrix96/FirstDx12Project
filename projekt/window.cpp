#include "window.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				DestroyWindow(hWnd);
			}
		}
		return 0;

	case WM_DESTROY:	// x button on top right corner of window was pressed
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

Window::Window()
{
	this->screenHeight = 0;
	this->screenWidth = 0;
	this->viewport = {};
	this->scissorRect = {};
	this->hInstance = (HINSTANCE)GetModuleHandle(NULL);
	this->hwnd = NULL;
	this->wcex = { 0 };
	this->camera = nullptr;
}

Window::~Window()
{
}

bool Window::Initialize(unsigned int width, unsigned int height)
{
	this->screenWidth = width;
	this->screenHeight = height;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"poopie doopie";
	if (!RegisterClassEx(&wcex))
	{
		return false;
	}

	RECT rc = { 0, 0, (long)screenWidth, (long)screenHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		L"poopie doopie",
		L"Dx12 3D Engine",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	ShowWindow(hwnd, SW_NORMAL);
	UpdateWindow(hwnd);

	this->camera = new Camera(this->screenWidth, this->screenHeight);

	return true;
}

void Window::CreateViewportAndScissorRect()
{
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = (long)0;
	scissorRect.right = (long)screenWidth;
	scissorRect.top = (long)0;
	scissorRect.bottom = (long)screenHeight;
}

unsigned int Window::GetScreenHeight()
{
	return this->screenHeight;
}

unsigned int Window::GetScreenWidth()
{
	return this->screenWidth;
}

D3D12_VIEWPORT* Window::GetViewport()
{
	return &viewport;
}

D3D12_RECT* Window::GetRect()
{
	return &scissorRect;
}

HWND* Window::GetHwnd()
{
	return &hwnd;
}

Camera* Window::GetCamera()
{
	return this->camera;
}

void Window::SetTitle(std::string title)
{
	SetWindowTextA(this->hwnd, (LPCSTR)title.c_str());
}
