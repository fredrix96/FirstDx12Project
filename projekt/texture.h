#pragma once
#include <d3d12.h>
#include <wincodec.h>
#include <string>
#include "d3dx12.h"
#include <DirectXMath.h>

using namespace DirectX;

class Texture
{
public:
	Texture();
	~Texture();

	int LoadFromFile(char* filename, ID3D12Device5* device);
	void Bind(ID3D12GraphicsCommandList4* commandList);
	void BindMulti(ID3D12GraphicsCommandList4* commandList);

	int LoadImageDataFromFile(LPCWSTR filename);
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

	ID3D12Resource* GetTextureBuffer();
	ID3D12Resource* GetTextureBufferArray(int pos);

	void CreateHeap(ID3D12Device5* device);

	ID3D12DescriptorHeap* GetTextureDescriptorHeap();

	void BindlessTechnique(std::vector<std::string> textureVec, ID3D12Device5* device);

	int GetVecSize();

private:
	ID3D12DescriptorHeap* descriptorHeap;
	ID3D12Resource* textureBuffer; // the resource heap containing our texture
	ID3D12Resource* textureBufferUploadHeap;
	D3D12_SUBRESOURCE_DATA textureData = {};
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	D3D12_RESOURCE_DESC textureDesc;

	UINT64 textureUploadBufferSize;
	int imageBytesPerRow;
	BYTE* imageData;
	int imageSize;

	/*------------ BINDLESS STUFF -------------*/
	//UINT64 textureUploadBufferSize;
	std::vector<int> imageBytesPerRowVec;
	std::vector<BYTE*> imageDataVec;
	std::vector<ID3D12Resource*> textureBufferVec;
	std::vector<ID3D12Resource*> textureBufferUploadHeapVec;
	std::vector<std::wstring> texVec;

	BYTE** textureImageData;
	UINT textureChannelCount = 4;
	UINT subresourceCount;
	UINT64 uploadBufferStep;
	UINT64 uploadBufferSize;
};