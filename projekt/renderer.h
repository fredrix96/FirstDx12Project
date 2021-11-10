#pragma once
#include <list>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "window.h"
#include "object.h"
#include <vector>
#include <string>
#include "d3dx12.h"
#include "D3D12Timer.h"
#include <iostream>

const unsigned int NUM_SWAP_BUFFERS = 2;

template<class Interface>
inline void SafeRelease(
	Interface** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Initialize();
	void CreateDirect3DDevice();
	void CreateCommandInterfacesAndSwapChain();
	void CreateFenceAndEventHandle();
	void CreateRenderTargets();
	void CreateConstantBufferResources();
	void CreateDepthStencil();

	void CreateRootSignature(ID3D12Device5* device);

	void Frame();
	void WaitForGpu();

	Window* GetWindow();
	Camera* GetCamera();
	Object* GetObj(int pos);
	int GetNumObjects();
	void SetTimer();

	void CreateObject(bool wireframe, XMFLOAT4 pos, float* scale, std::string path);
	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);
	void SetClearColor(float r, float g, float b, float a);

	// benchmarking
	void BenchmarkObjects();
	void BenchmarkFrame();

private:
	ID3D12RootSignature* rootSignature;

	std::vector<Object> objects;

	ID3D12GraphicsCommandList4* commandList;
	ID3D12CommandQueue* commandQueue;
	ID3D12CommandAllocator* commandAllocator;

	ID3D12Device5* device;
	IDXGISwapChain4* swapChain;
	HRESULT hr;

	ID3D12Fence1* fence;
	HANDLE eventHandle;
	UINT64 fenceValue;

	ID3D12DescriptorHeap* renderTargetsHeap;
	ID3D12Resource1* renderTargets[NUM_SWAP_BUFFERS];
	UINT renderTargetDescriptorSize;

	ID3D12DescriptorHeap* descriptorHeap[NUM_SWAP_BUFFERS];
	ID3D12Resource1* constantBufferResource[NUM_SWAP_BUFFERS];

	ID3D12Resource1* depthStencilBuffer[NUM_SWAP_BUFFERS];
	ID3D12DescriptorHeap* dsDescriptorHeap;
	UINT dsDescriptorHeapSize;

	Window window;
	UINT backBufferIndex;

	float clearColor[4] = { 0,0,0,0 };

	bool firstFrame = true;

	int texInd = 0;
	int savedInd = 0;
	int herz = 0;

	D3D12::D3D12Timer gpuTimerObj;
	D3D12::D3D12Timer gpuTimerFrame;
	std::vector<double> benchmarkVecObj;
	std::vector<double> benchmarkVecFrame;
	int benchmarkSamples = 1000;
};