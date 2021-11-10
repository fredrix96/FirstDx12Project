#include "renderer.h"

// window size
#define WIDTH 1920
#define HEIGHT 1080

void run();
void updateScene();
void renderScene();

Renderer renderer;

int main()
{
	//----------------Initialization--------------------//
	renderer.GetWindow()->Initialize(WIDTH, HEIGHT);
	renderer.Initialize();
	renderer.SetClearColor(0.0, 0.0, 0.25, 1.0);

	XMFLOAT4 pos = XMFLOAT4(0.0, 0.0, 0.0, 0.0);
	float scale[3] = { 0.0, 0.0, 0.0 };
	std::string path = "";
	//--------------------------------------------------//

	/*--------- big box with dynamic texture ----------*/
	pos = XMFLOAT4(0, 0, 0, 0);
	scale[0] = 10;
	scale[1] = 10;
	scale[2] = 10;
	path = "../objects/box.obj";
	renderer.CreateObject(false, pos, scale, path);
	/*--------------------------------------------------*/

	/*-------- small boxes with dynamic texture --------*/
	pos = XMFLOAT4(2, 0, 0, 0);
	scale[0] = 1;
	scale[1] = 1;
	scale[2] = 1;
	path = "../objects/box.obj";
	renderer.CreateObject(false, pos, scale, path);

	pos = XMFLOAT4(-2, 0, 0, 0);
	renderer.CreateObject(false, pos, scale, path);
	/*--------------------------------------------------*/

	/*-------- big piedmon with dynamic texture --------*/
	pos = XMFLOAT4(0, 0, 0, 0);
	scale[0] = 1;
	scale[1] = 1;
	scale[2] = 1;
	path = "../objects/piedmonGif.obj";
	renderer.CreateObject(false, pos, scale, path);
	/*--------------------------------------------------*/

	/*------- two piedmons standing on the boxes -------*/
	pos = XMFLOAT4(2, 1, 0, 0);
	scale[0] = 0.5;
	scale[1] = 0.5;
	scale[2] = 0.5;
	path = "../objects/piedmon.obj";
	renderer.CreateObject(false, pos, scale, path);

	pos = XMFLOAT4(-2, 1, 0, 0);
	renderer.CreateObject(false, pos, scale, path);
	/*--------------------------------------------------*/

	renderer.SetTimer();
	run();

	//print benchmarks in console after window closes
	renderer.BenchmarkObjects();	// per object
	renderer.BenchmarkFrame();		// whole frame

	return 0;
}

void run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Start clock
			renderer.GetCamera()->BeginFrame();

			// run game code
			updateScene();	// update game logic
			renderScene();	// execute the command queue (rendering the scene is the result of the gpu executing the command lists)

			// Stop clock
			renderer.GetCamera()->EndFrame();
			std::string time = renderer.GetCamera()->GetFPS();
			renderer.GetWindow()->SetTitle(time);
		}
	}
}

void updateScene()
{
	renderer.GetCamera()->MouseMovement();
	renderer.GetCamera()->KeyMovement();

	// loop per object
	for (int i = 0; i < renderer.GetNumObjects(); i++)
	{
		// update app logic, such as moving the camera or figuring out what objects are in view

		// create rotation matrices
		XMMATRIX rotXMat = XMMatrixRotationX(0.0000f);
		XMMATRIX rotYMat = XMMatrixRotationY(0.0000f);
		XMMATRIX rotZMat = XMMatrixRotationZ(0.0000f);

		// add rotation to objects rotation matrix and store it
		XMMATRIX rotMat = XMLoadFloat4x4(renderer.GetObj(i)->GetRotMatrix()) * rotXMat * rotYMat * rotZMat;
		renderer.GetObj(i)->SetRotMatrix(rotMat);

		// create translation matrix for object from its position vector
		XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(renderer.GetObj(i)->GetPosition()));

		XMMATRIX scale = XMMatrixScaling(renderer.GetObj(i)->GetScale()[0], renderer.GetObj(i)->GetScale()[1], renderer.GetObj(i)->GetScale()[2]);

		// create world matrix by first rotating the object, then positioning the rotated cube
		XMMATRIX worldMat = scale * rotMat * translationMat;

		// store world matrix
		renderer.GetObj(i)->SetWorldMatrix(worldMat);

		// update constant buffer
		// create the wvp matrix and store in constant buffer
		XMMATRIX viewMat = XMLoadFloat4x4(renderer.GetCamera()->GetCamViewMat()); // load view matrix
		XMMATRIX projMat = XMLoadFloat4x4(renderer.GetCamera()->GetCamProjMat()); // load projection matrix
		XMMATRIX wvpMat = XMLoadFloat4x4(renderer.GetObj(i)->GetWorldMatrix()) * viewMat * projMat; // create wvp matrix
		XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
		renderer.GetObj(i)->GetConstantBuffer()->SetWvpMat(transposed);
	}
}

void renderScene()
{
	renderer.Frame();
}
