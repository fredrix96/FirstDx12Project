#pragma once
#include "constantBuffer.h"
#include "vertexbuffer.h"
#include <vector>
#include <string>
#include <D3Dcompiler.h>
#include "texture.h"

#define MATRIXSIZE 16

class Object
{
public:
	Object();
	~Object();

	void InitializeMatrices();

	void CreateConstantBuffer();
	void CreateVertexBuffer(ID3D12Device5* device, float* data, size_t size);
	void CreateMaterials(ID3D12Device5* device, bool wireframe, ID3D12RootSignature* rootSignature);
	bool CreateShaders();
	bool CreatePSO(ID3D12Device5* device, bool wireframe, ID3D12RootSignature* rootSignature);

	ConstantBuffer* GetConstantBuffer();
	VertexBuffer* GetVertexBuffer(int index);

	ID3D12PipelineState* GetPipeLineState();

	XMFLOAT4* GetPosition();
	float* GetScale();
	XMFLOAT4X4* GetRotMatrix();
	XMFLOAT4X4* GetWorldMatrix();
	int GetNrOfVertices();
	Texture* GetTexture();

	void SetScale(float* scale);
	void SetPosition(XMFLOAT4 pos);
	void SetRotMatrix(XMMATRIX rotMat);
	void SetWorldMatrix(XMMATRIX worldMat);

	void LoadObj(std::string path, ID3D12Device5* device);
	void LoadMtl(const char* mtlPath);

private:
	XMFLOAT4 position;
	float scale[3];

	XMFLOAT4X4 worldMat;
	XMFLOAT4X4 rotMat; 
	XMVECTOR posVec;

	ConstantBuffer* constantBuffer;
	std::vector<VertexBuffer*> vertexBuffers;

	ID3DBlob* VSshader;
	ID3DBlob* PSshader;

	ID3D12PipelineState* pipeLineState;

	std::vector<XMFLOAT3> out_vertices;
	std::vector<float> dataVector;

	char material[50];
	char textureName[50];
	std::vector<XMFLOAT2> out_UVs;
	std::vector<float> uvVector;

	Texture* texture;
	std::vector<std::string>textureVec;
};