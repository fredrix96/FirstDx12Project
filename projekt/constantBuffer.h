#pragma once
#include <map>
#include <DirectXMath.h>
#include <d3d12.h>

using namespace DirectX;

static enum types
{
	Positions,
	UV,
	WVP,
	TextureDT,
	TextureIndex,
	TYPE_SIZE
};

class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	float* GetBuffData(int type);
	XMFLOAT4X4* GetWvpMat();

	void SetData(int type, float* data);
	void SetWvpMat(XMMATRIX mat);
	
private:
	// Key, data
	std::map<int, float*> buffData;

	XMFLOAT4X4 wvpMat;

	ID3D12Resource* constantBufferUploadHeaps[2]; // this is the memory on the gpu where constant buffers for each frame will be placed

	UINT8* cbvGPUAddress[2]; // this is a pointer to each of the constant buffer resource heaps
};