#pragma once
#include <windows.h>
#include <d3d12.h>
#include "camera.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Window
{
public:
	Window();
	~Window();

	bool Initialize(unsigned int width, unsigned int height);
	void CreateViewportAndScissorRect();
	unsigned int GetScreenHeight();
	unsigned int GetScreenWidth();
	D3D12_VIEWPORT* GetViewport();
	D3D12_RECT* GetRect();
	HWND* GetHwnd();
	Camera* GetCamera();
	void SetTitle(std::string title);

private:
	unsigned int screenHeight;
	unsigned int screenWidth;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	WNDCLASSEX wcex;
	HINSTANCE hInstance;
	HWND hwnd;
	Camera* camera;
};