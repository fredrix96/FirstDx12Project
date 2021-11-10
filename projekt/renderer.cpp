#include "renderer.h"

Renderer::Renderer()
{
	this->herz = 60;
}

Renderer::~Renderer()
{
}

void Renderer::Initialize()
{
	CreateDirect3DDevice();
	CreateCommandInterfacesAndSwapChain();
	CreateFenceAndEventHandle();
	CreateRenderTargets();
	CreateConstantBufferResources();
	CreateDepthStencil();
	this->window.CreateViewportAndScissorRect();
	CreateRootSignature(this->device);
}

void Renderer::CreateDirect3DDevice()
{
	ID3D12Debug* dbg;
	D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&dbg);
	dbg->EnableDebugLayer();
	dbg->Release();

	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIFactory6* factory = nullptr;
	IDXGIAdapter1* adapter = nullptr;

	//First a factory is created to iterate through the adapters available.
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	for (UINT adapterIndex = 0;; ++adapterIndex)
	{
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter))
		{
			break; //No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device5), nullptr)))
		{
			break;
		}

		SafeRelease(&adapter);
	}
	if (adapter)
	{
		hr = S_OK;

		//Create the actual device.
		if (SUCCEEDED(hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device))))
		{
			OutputDebugStringA("Device created!\n");
		}

		SafeRelease(&adapter);
	}
	else
	{
		//Create warp device if no adapter was found.
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
		OutputDebugStringA("No adapter found! Warp device created!\n");
	}

	SafeRelease(&factory);
}

void Renderer::CreateCommandInterfacesAndSwapChain()
{
	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	if (!SUCCEEDED(hr = device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&commandQueue))))
	{
		OutputDebugStringA("ERROR: Could not create Command Queue!\n");
	}

	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	if (!SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator))))
	{
		OutputDebugStringA("ERROR: Could not create Command Allocator!\n");
	}

	//Create command list.
	if (!SUCCEEDED(hr = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator,
		nullptr,
		IID_PPV_ARGS(&commandList))))
	{
		OutputDebugStringA("ERROR: Could not create Commandlist!\n");
	}

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	commandList->Close();

	IDXGIFactory5* factory = nullptr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));

	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = NUM_SWAP_BUFFERS;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;
	if (SUCCEEDED(factory->CreateSwapChainForHwnd(
		commandQueue,
		*this->window.GetHwnd(),
		&scDesc,
		nullptr,
		nullptr,
		&swapChain1)))
	{
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain))))
		{
			swapChain->Release();
		}
	}
	else
	{
		OutputDebugStringA("ERROR: Could not create Swap Chain!\n");
	}

	SafeRelease(&factory);
}

void Renderer::CreateFenceAndEventHandle()
{
	if (!SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
	{
		OutputDebugStringA("ERROR: Could not create Fence!\n");
	}
	fenceValue = 1;

	//Create an event handle to use for GPU synchronization.
	eventHandle = CreateEvent(0, false, false, 0);
}

void Renderer::CreateRenderTargets()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if (!SUCCEEDED(hr = device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&renderTargetsHeap))))
	{
		OutputDebugStringA("ERROR: Could not create Render Target Heap!\n");
	}

	//Create resources for the render targets.
	renderTargetDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		if (SUCCEEDED(hr = swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]))))
		{
			device->CreateRenderTargetView(renderTargets[n], nullptr, cdh);
			cdh.ptr += renderTargetDescriptorSize;
		}
		else
		{
			OutputDebugStringA("ERROR: Could not create Render Target View!\n");
		}
	}
}

void Renderer::CreateConstantBufferResources()
{
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 1;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		if (!SUCCEEDED(device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&descriptorHeap[i]))))
		{
			OutputDebugStringA("ERROR: Could not create Descriptor Heap");
		}
	}

	UINT cbSizeAligned = (sizeof(ConstantBuffer) + 255) & ~255;	// 256-byte aligned CB.

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1; //used when multi-gpu
	heapProperties.VisibleNodeMask = 1; //used when multi-gpu
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Create a resource heap, descriptor heap, and pointer to cbv for each frame
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constantBufferResource[i])
			);

		constantBufferResource[i]->SetName(L"cb heap");

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = constantBufferResource[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = cbSizeAligned;
		device->CreateConstantBufferView(&cbvDesc, descriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
	}
}

void Renderer::CreateDepthStencil()
{
	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 2;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));
	if (FAILED(hr))
	{
		OutputDebugStringA("Could not create descriptorheap for depthstencil");
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	dsDescriptorHeapSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	D3D12_CPU_DESCRIPTOR_HANDLE dsh = dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		if (SUCCEEDED(hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, window.GetScreenWidth(), window.GetScreenHeight(), 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&depthStencilBuffer[n])
			)))
		{
			device->CreateDepthStencilView(depthStencilBuffer[n], &depthStencilDesc, dsh);
			dsh.ptr += dsDescriptorHeapSize;
		}
	}

	dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");
}

void Renderer::CreateRootSignature(ID3D12Device5* device)
{
	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  dtRanges[1];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	dtRanges[0].NumDescriptors = UINT_MAX; // for bindless
	dtRanges[0].BaseShaderRegister = 0; // Slot where the texture is included in Pixel Shader
	dtRanges[0].RegisterSpace = 0;
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	//create root parameter
	D3D12_ROOT_PARAMETER rootParam[TYPE_SIZE]{};

	rootParam[Positions].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParam[Positions].Descriptor.ShaderRegister = Positions;
	rootParam[Positions].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootParam[UV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParam[UV].Descriptor.ShaderRegister = UV;
	rootParam[UV].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// create a root parameter for wvp matrix
	rootParam[WVP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam[WVP].Constants.ShaderRegister = WVP;
	rootParam[WVP].Constants.Num32BitValues = 16;
	rootParam[WVP].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// Texture
	rootParam[TextureDT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[TextureDT].DescriptorTable = dt;
	rootParam[TextureDT].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// create a root parameter for TextureIndex
	rootParam[TextureIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam[TextureIndex].Constants.ShaderRegister = TextureIndex;
	rootParam[TextureIndex].Constants.Num32BitValues = 1;
	rootParam[TextureIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// create a static sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &sampler;

	ID3DBlob* sBlob;
	D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr);

	device->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
}

void Renderer::Frame()
{
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	commandAllocator->Reset();
	if (!SUCCEEDED(hr = commandList->Reset(commandAllocator, NULL)))
	{
		OutputDebugStringA("ERROR: Could not reset commandlist!\n");
	}

	if (!firstFrame)
	{
		//benchmark
		gpuTimerFrame.stop(commandList, 0);

		gpuTimerFrame.resolveQueryToCPU(commandList, 0);

		//get time in ms
		UINT64 queueFreq;
		commandQueue->GetTimestampFrequency(&queueFreq);
		double timestampToMs = (1.0 / queueFreq) * 1000.0;
		D3D12::GPUTimestampPair drawTime = gpuTimerFrame.getTimestampPair(0);
		UINT64 dt = drawTime.Stop - drawTime.Start;
		double timeInMs = dt * timestampToMs;
		if ((benchmarkVecFrame.size() < benchmarkSamples) && (firstFrame == false))
		{
			benchmarkVecFrame.push_back(timeInMs);
		}
	}

	//benchmark
	gpuTimerFrame.start(commandList, 0);

	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap[backBufferIndex] };
	commandList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	//Set necessary states.
	commandList->RSSetViewports(1, window.GetViewport());
	commandList->RSSetScissorRects(1, window.GetRect());

	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(commandList,
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
		);

	//Record commands.
	//Get the handle for the current render target used as back buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += renderTargetDescriptorSize * backBufferIndex;

	D3D12_CPU_DESCRIPTOR_HANDLE dsh = dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	dsh.ptr += dsDescriptorHeapSize * backBufferIndex;

	commandList->OMSetRenderTargets(1, &cdh, true, &dsh);

	// Clear the render target by using the ClearRenderTargetView command
	commandList->ClearRenderTargetView(cdh, clearColor, 0, nullptr);

	// clear the depth/stencil buffer
	commandList->ClearDepthStencilView(dsh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//Set root signature
	commandList->SetGraphicsRootSignature(this->rootSignature);

	Object* object;
	for (int i = 0; i < GetNumObjects(); i++)
	{
		object = &objects.at(i);

		// First frame? Bind the textures
		if (firstFrame && object->GetTexture()->GetVecSize() > 0)
		{
			object->GetTexture()->BindMulti(commandList);
		}
		else if (firstFrame)
		{
			object->GetTexture()->Bind(commandList);
		}

		// Create texture buffer descriptor heap
		ID3D12DescriptorHeap* textureDescriptorHeaps[] = { object->GetTexture()->GetTextureDescriptorHeap() };

		commandList->SetDescriptorHeaps(ARRAYSIZE(textureDescriptorHeaps), textureDescriptorHeaps);

		commandList->SetPipelineState(object->GetPipeLineState());

		if (object->GetTexture()->GetVecSize() > 0)
		{
			if (GetCamera()->GetAccumulatedTime() > (1000 / this->herz)) // 60 images per second (60HZ)
			{
				savedInd++;
				GetCamera()->ResetAccumulatedTime();
			}

			this->texInd = savedInd;
			if (texInd > object->GetTexture()->GetVecSize())
			{
				savedInd = 0;
				texInd = 0;
			}

			for (int j = 0; j < object->GetTexture()->GetVecSize(); j++)
			{
				// transition the texture default heap to a pixel shader resource
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(object->GetTexture()->GetTextureBufferArray(j), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			}
		}
		else
		{
			this->texInd = 0;
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(object->GetTexture()->GetTextureBuffer(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}

		commandList->SetGraphicsRootDescriptorTable(TextureDT, object->GetTexture()->GetTextureDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->SetGraphicsRootShaderResourceView(Positions, object->GetVertexBuffer(Positions)->GetVertexBufferResource()->GetGPUVirtualAddress());
		commandList->SetGraphicsRootShaderResourceView(UV, object->GetVertexBuffer(UV)->GetVertexBufferResource()->GetGPUVirtualAddress());

		XMFLOAT4X4* wvp = object->GetConstantBuffer()->GetWvpMat();
		commandList->SetGraphicsRoot32BitConstants(WVP, MATRIXSIZE, wvp, 0);
		commandList->SetGraphicsRoot32BitConstants(TextureIndex, 1, &texInd, 0);

		gpuTimerObj.start(commandList, i);
		commandList->DrawInstanced(object->GetNrOfVertices(), 1, 0, 0);
		gpuTimerObj.stop(commandList, i);
		gpuTimerObj.resolveQueryToCPU(commandList, i);

		if (object->GetTexture()->GetVecSize() > 0)
		{
			for (int j = 0; j < object->GetTexture()->GetVecSize(); j++)
			{
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(object->GetTexture()->GetTextureBufferArray(j), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
			}
		}
		else
		{
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(object->GetTexture()->GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
		}

		//get time in ms
		UINT64 queueFreq;
		commandQueue->GetTimestampFrequency(&queueFreq);
		double timestampToMs = (1.0 / queueFreq) * 1000.0;
		D3D12::GPUTimestampPair drawTime = gpuTimerObj.getTimestampPair(i);
		UINT64 dt = drawTime.Stop - drawTime.Start;
		double timeInMs = dt * timestampToMs;
		// get benchmarkSamples benchmarks per object
		if ((benchmarkVecObj.size() < GetNumObjects() * benchmarkSamples) && (firstFrame == false))
		{
			benchmarkVecObj.push_back(timeInMs);
		}
		///*std::string title = std::to_string(timeInMs) + std::string("ms");
		//std::cout << "Object: " << i << "		Time: " << title << std::endl;*/
	}

	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(commandList,
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
		);

	//Close the list to prepare it for execution.
	if (!SUCCEEDED(hr = commandList->Close()))
	{
		OutputDebugStringA("ERROR: Could not close commandlist!\n");
	}

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { commandList };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	swapChain->Present1(0, 0, &pp);

	WaitForGpu(); //Wait for GPU to finish.

	if (firstFrame)
	{
		firstFrame = false;
	}
}

void Renderer::WaitForGpu()
{
	//Signal and increment the fence value.
	const UINT64 fenceV = fenceValue;
	commandQueue->Signal(fence, fenceV);
	fenceValue++;

	//Wait until command queue is done.
	if (fence->GetCompletedValue() < fenceV)
	{
		fence->SetEventOnCompletion(fenceV, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
	}
}

Window* Renderer::GetWindow()
{
	return &window;
}

Camera* Renderer::GetCamera()
{
	return this->window.GetCamera();
}

int Renderer::GetNumObjects()
{
	return this->objects.size();
}

void Renderer::SetTimer()
{
	// benchmarking
	gpuTimerObj.init(this->device, GetNumObjects());
	gpuTimerFrame.init(this->device, 1);
}

void Renderer::CreateObject(bool wireframe, XMFLOAT4 pos, float* scale, std::string path)
{
	Object object;
	object.SetPosition(pos);
	object.SetScale(scale);
	object.InitializeMatrices();

	//object loader...
	object.LoadObj(path, this->device);

	object.CreateConstantBuffer();
	object.CreateMaterials(this->device, wireframe, this->rootSignature);

	objects.push_back(object);
}

Object* Renderer::GetObj(int pos)
{
	return &objects.at(pos);
}

void Renderer::SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
	clearColor[0] = r;
	clearColor[1] = g;
	clearColor[2] = b;
	clearColor[3] = a;
}

void Renderer::BenchmarkObjects()
{
	std::vector<double> sumVec;
	sumVec.resize(GetNumObjects());

	//get benchmarks for drawcall per object
	for (int i = 0; i < GetNumObjects(); i++)
	{
		for (int j = 0; j < GetNumObjects() * benchmarkSamples; j += GetNumObjects())
		{
			sumVec.at(i) += benchmarkVecObj.at(j + i);
		}
	}

	//print averages
	for (int i = 0; i < GetNumObjects(); i++)
	{
		std::cout << "Obj nr: " << i << ": " << sumVec.at(i) / benchmarkSamples << std::endl;
	}
	std::cout << "Benchmark average was made with " << benchmarkSamples << " samples" << std::endl;
}

void Renderer::BenchmarkFrame()
{
	double sum = 0;
	for (int i = 0; i < benchmarkSamples; i++)
	{
		sum += benchmarkVecFrame.at(i);
	}
	std::cout << "Average benchmark in ms for frame: " << sum / benchmarkSamples << std::endl;
	std::cout << "Benchmark average was made with " << benchmarkSamples << " samples" << std::endl;
}