#include "vertexbuffer.h"

VertexBuffer::VertexBuffer(ID3D12Device5* device, float* data, size_t size)
{
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = size;
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Creates both a resource and an implicit heap, such that the heap is big enough
	//to contain the entire resource and the resource is mapped to the heap. 
	HRESULT hr;
	if (!SUCCEEDED(hr = device->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferResource))))
	{
		OutputDebugStringA("ERROR: Could not create Commited Resource");
	}

	vertexBufferResource->SetName(L"vb heap");

	//Copy the triangle data to the vertex buffer.
	unsigned char* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 }; //We do not intend to read this resource on the CPU.
	vertexBufferResource->Map(0, &range, (void**)&dataBegin);

	// the offset is not necessary in this case!
	// could be great if we have a dataset of {position, normal, uv} but we have them separately 
	memcpy(dataBegin, data, size);

	vertexBufferResource->Unmap(0, nullptr);
}

VertexBuffer::~VertexBuffer()
{
}

ID3D12Resource1* VertexBuffer::GetVertexBufferResource()
{
	return this->vertexBufferResource;
}
