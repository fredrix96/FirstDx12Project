#include "object.h"

#pragma warning (disable: 4996)

Object::Object()
{
	constantBuffer = nullptr;
	vertexBuffers = {};
	VSshader = nullptr;
	PSshader = nullptr;
	pipeLineState = nullptr;
	texture = new Texture();
}

Object::~Object()
{
}

void Object::InitializeMatrices()
{
	posVec = XMLoadFloat4(&this->position);
	XMMATRIX tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from object's position vector
	XMStoreFloat4x4(&rotMat, XMMatrixIdentity()); // initialize object's rotation matrix to identity matrix
	XMStoreFloat4x4(&worldMat, tmpMat); // store object's world matrix
}

void Object::CreateConstantBuffer()
{
	constantBuffer = new ConstantBuffer();
}

void Object::CreateVertexBuffer(ID3D12Device5* device, float* data, size_t size)
{
	VertexBuffer* vb = new VertexBuffer(device, data, size);
	vertexBuffers.push_back(vb);
}

ConstantBuffer* Object::GetConstantBuffer()
{
	return constantBuffer;
}

VertexBuffer* Object::GetVertexBuffer(int index)
{
	return this->vertexBuffers.at(index);
}

ID3D12PipelineState* Object::GetPipeLineState()
{
	return this->pipeLineState;
}

XMFLOAT4* Object::GetPosition()
{
	return &this->position;
}

float* Object::GetScale()
{
	return this->scale;
}

XMFLOAT4X4* Object::GetRotMatrix()
{
	return &this->rotMat;
}

XMFLOAT4X4* Object::GetWorldMatrix()
{
	return &this->worldMat;
}

int Object::GetNrOfVertices()
{
	return this->dataVector.size() / 3;
}

Texture* Object::GetTexture()
{
	return this->texture;
}

void Object::SetScale(float* scale)
{
	this->scale[0] = scale[0];
	this->scale[1] = scale[1];
	this->scale[2] = scale[2];
}

void Object::SetPosition(XMFLOAT4 pos)
{
	this->position = pos;
}

void Object::SetRotMatrix(XMMATRIX rotMat)
{
	XMStoreFloat4x4(&this->rotMat, rotMat);
}

void Object::SetWorldMatrix(XMMATRIX worldMat)
{
	XMStoreFloat4x4(&this->worldMat, worldMat);
}


void Object::LoadObj(std::string objPath, ID3D12Device5* device)
{
	std::vector<unsigned int> vertexIndices, UVIndices, normalIndices;
	std::vector<XMFLOAT3> tmp_vertices, tmp_normals, out_normals;
	std::vector<XMFLOAT2> tmp_UVs;

	FILE* file = fopen(objPath.c_str(), "r");

	if (file == NULL) {
		printf("ERROR LOADING OBJECT! \n");
	}

	while (1) { //runs until a break
				//reads the header of the file and assumes that
				//the first word wont be longer than 100
		char header[100];
		int res = fscanf(file, "%s", header);
		if (res == EOF) {
			break;
		}
		else {
			//if the first word of the line starts with "mtllib", then we will copy the string name
			//of the mtl file
			if (strcmp(header, "mtllib") == 0) {
				fscanf(file, "%s\n", &this->material);
			}
			//if the first word of the line is "v", then the rest
			//has to be 3 floats. So we will create a vec3 out of them
			//and add it to our vector
			else if (strcmp(header, "v") == 0) {
				XMFLOAT3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				tmp_vertices.push_back(vertex); //adds vertex to the end of our vector
			}
			//same with the UVs, but with just two floats and only if the line starts with "vt"
			else if (strcmp(header, "vt") == 0) {
				XMFLOAT2 UV;
				fscanf(file, "%f %f\n", &UV.x, &UV.y);
				//have to flip the y-coordinates because openGl starts at the bottom
				UV.y = 1.0f - UV.y;
				tmp_UVs.push_back(UV);
			}
			//same with the normals
			else if (strcmp(header, "vn") == 0) {
				XMFLOAT3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				tmp_normals.push_back(normal);
			}
			//finally doing the same with the lines starting with "f". Although the process is
			//similar to the others, this one needs you to look if the line matches with the amount
			//of data necessary to assign the unsigned ints.
			else if (strcmp(header, "f") == 0) {
				std::string v1, v2, v3;
				unsigned int vertexIndex[3], UVIndex[3], normalIndex[3];
				int amount = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
					&vertexIndex[0], &UVIndex[0], &normalIndex[0],
					&vertexIndex[1], &UVIndex[1], &normalIndex[1],
					&vertexIndex[2], &UVIndex[2], &normalIndex[2]);
				if (amount != 9) {
					printf("ERROR! The object can not be read by the parser!\n");
				}
				for (unsigned int i = 0; i < 3; i++) {
					vertexIndices.push_back(vertexIndex[i]);
					UVIndices.push_back(UVIndex[i]);
					normalIndices.push_back(normalIndex[i]);
				}
			}
		}
	}

	//we have now changed the shape of the incoming data to vectors, but now we need to
	//change the data from vector to vec3 which OpenGL is used to (Indexing). 

	//we need to go trough each vertex (v/vt/vn) of each triangle (f)
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		unsigned int vertexIndex = vertexIndices[i];

		//OBJ indexing starts at 1 while C++ starts at 0, therefore we need to substract 1
		XMFLOAT3 vertex = tmp_vertices[vertexIndex - 1];
		out_vertices.push_back(vertex);

		//same with the UVs
		unsigned int UVIndex = UVIndices[i];
		XMFLOAT2 UV = tmp_UVs[UVIndex - 1];
		out_UVs.push_back(UV);

		//same with the normals
		unsigned int normalIndex = normalIndices[i];
		XMFLOAT3 normal = tmp_normals[normalIndex - 1];
		out_normals.push_back(normal);
	}

	// move data into float format
	for (int i = 0; i < out_vertices.size(); i++)
	{
		dataVector.push_back(out_vertices.at(i).x);
		dataVector.push_back(out_vertices.at(i).y);
		dataVector.push_back(out_vertices.at(i).z);
	}

	// Creating Vertex Buffer
	CreateVertexBuffer(device, &dataVector[0], sizeof(float) * dataVector.size());

	// move data into float format
	for (int i = 0; i < out_UVs.size(); i++)
	{
		uvVector.push_back(out_UVs.at(i).x);
		uvVector.push_back(out_UVs.at(i).y);
	}

	CreateVertexBuffer(device, &uvVector[0], sizeof(float) * uvVector.size());

	LoadMtl(material);

	// Creating Texture
	if (this->textureVec.size() == 1)
	{
		texture->LoadFromFile(this->textureName, device);
	}
	else
	{
		texture->BindlessTechnique(this->textureVec, device);
	}
}

void Object::LoadMtl(const char* mtlPath)
{
	int counter = 0;
	std::string patath = "../objects/";
	while (mtlPath[counter] != NULL)
	{
		patath.push_back(mtlPath[counter]);
		counter++;
	}

	FILE* file = fopen(patath.c_str(), "r");

	if (file == NULL) {
		printf("ERROR LOADING MATERIAL! \n");
	}

	while (1) { //runs until a break
				//reads the header of the file and assumes that
				//the first word wont be longer than 100
		char header[100];
		int res = fscanf(file, "%s", header);
		if (res == EOF) {
			break;
		}
		else {
			//here we find the texture (material) to the object
			if (strcmp(header, "map_Kd") == 0)
			{
				fscanf(file, "%s\n", &this->textureName);
				std::string texName = this->textureName;
				this->textureVec.push_back(texName);
			}
		}
	}
}

void Object::CreateMaterials(ID3D12Device5* device, bool wireframe, ID3D12RootSignature* rootSignature)
{
	CreateShaders();
	CreatePSO(device, wireframe, rootSignature);
}

bool Object::CreateShaders()
{
	// compile shaders
	ID3DBlob* errorBuff;

	HRESULT hr = D3DCompileFromFile(L"../shaders/VertexShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_1",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES,
		0,
		&VSshader,
		&errorBuff);

	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	hr = D3DCompileFromFile(L"../shaders/PixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_1",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES,
		0,
		&PSshader,
		&errorBuff);

	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	return true;
}

bool Object::CreatePSO(ID3D12Device5* device, bool wireframe, ID3D12RootSignature* rootSignature)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	//Specify pipeline stages:
	gpsd.pRootSignature = rootSignature;
	//gpsd.InputLayout = inputLayoutDesc; -- We dont have one!
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(VSshader->GetBufferPointer());
	gpsd.VS.BytecodeLength = VSshader->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(PSshader->GetBufferPointer());
	gpsd.PS.BytecodeLength = PSshader->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpsd.RasterizerState.FrontCounterClockwise = TRUE;

	gpsd.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpsd.DepthStencilState.DepthEnable = TRUE;
	gpsd.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsd.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

		// To be able to allow transparency
		gpsd.BlendState.RenderTarget[i].BlendEnable = TRUE;
		gpsd.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		gpsd.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	}

	if (wireframe == true)
	{
		gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	}
	else
	{
		gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	}

	if (!SUCCEEDED(device->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&pipeLineState))))
	{
		return false;
	}

	return true;
}
