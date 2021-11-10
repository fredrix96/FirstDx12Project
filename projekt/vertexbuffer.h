#pragma once
#include <d3d12.h>
#include <DirectXMath.h>

using namespace DirectX;

/*struct Vertex
{
	float x, y, z; // Position
	//float r, g, b; // Color
};*/

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 textureCoordinate;
};

class VertexBuffer
{
public:
	VertexBuffer(ID3D12Device5* device, float* data, size_t size);
	~VertexBuffer();

	ID3D12Resource1* GetVertexBufferResource();

private:
	size_t totalSize;

	D3D12_HEAP_PROPERTIES hp = {};
	D3D12_RESOURCE_DESC rd = {};

	ID3D12Resource1* vertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
};