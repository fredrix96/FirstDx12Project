#include "camera.h"

Camera::Camera(unsigned int width, unsigned int height)
{
	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovRH(80.0f * (3.14f / 180.0f), (float)width / (float)height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// set starting camera state
	cameraPosition = XMFLOAT4(0.0f, 1.5f, 4.0f, 0.0f);
	cameraDir = XMFLOAT4(0.0f, -0.3f, -1.0f, 0.0f);
	cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	UpdateViewMatrix();

	this->time = 0;
	this->deltaTime = 0;
	this->frames = 0;
	this->frameRate = 30;
	this->averageFrameTimeMilliseconds = 33.333;

	this->moveSpeed = 0.005;
	this->rotSpeed = 0.004;
	this->accumulatedTime = 0;
}

Camera::~Camera()
{
}

XMFLOAT4X4* Camera::GetCamProjMat()
{
	return &this->cameraProjMat;
}

XMFLOAT4X4* Camera::GetCamViewMat()
{
	return &this->cameraViewMat;
}

void Camera::MoveForward()
{
	this->cameraPosition.x += moveSpeed * this->cameraDir.x * this->time;
	this->cameraPosition.y += moveSpeed * this->cameraDir.y * this->time;
	this->cameraPosition.z += moveSpeed * this->cameraDir.z * this->time;

	UpdateViewMatrix();
}

void Camera::MoveBackward()
{
	this->cameraPosition.x += -moveSpeed * this->cameraDir.x * this->time;
	this->cameraPosition.y += -moveSpeed * this->cameraDir.y * this->time;
	this->cameraPosition.z += -moveSpeed * this->cameraDir.z * this->time;

	UpdateViewMatrix();
}

void Camera::MoveRight()
{
	XMVECTOR xAxis = XMVector3Cross(XMLoadFloat4(&this->cameraDir), XMLoadFloat4(&this->cameraUp));
	this->cameraPosition.x += moveSpeed * XMVectorGetX(xAxis) * this->time;
	this->cameraPosition.y += moveSpeed * XMVectorGetY(xAxis) * this->time;
	this->cameraPosition.z += moveSpeed * XMVectorGetZ(xAxis) * this->time;

	UpdateViewMatrix();

}

void Camera::MoveLeft()
{
	XMVECTOR xAxis = XMVector3Cross(XMLoadFloat4(&this->cameraDir), XMLoadFloat4(&this->cameraUp));
	this->cameraPosition.x += -moveSpeed * XMVectorGetX(xAxis) * this->time;
	this->cameraPosition.y += -moveSpeed * XMVectorGetY(xAxis) * this->time;
	this->cameraPosition.z += -moveSpeed * XMVectorGetZ(xAxis) * this->time;

	UpdateViewMatrix();
}

void Camera::MoveUp()
{
	this->cameraPosition.x += moveSpeed * this->cameraUp.x * this->time;
	this->cameraPosition.y += moveSpeed * this->cameraUp.y * this->time;
	this->cameraPosition.z += moveSpeed * this->cameraUp.z * this->time;

	UpdateViewMatrix();
}

void Camera::MoveDown()
{
	this->cameraPosition.x += -moveSpeed * this->cameraUp.x * this->time;
	this->cameraPosition.y += -moveSpeed * this->cameraUp.y * this->time;
	this->cameraPosition.z += -moveSpeed * this->cameraUp.z * this->time;

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cDir = XMLoadFloat4(&cameraDir);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	XMMATRIX tmpMat = XMMatrixLookToRH(cPos, cDir, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);
}

void Camera::MouseUpdate(float x, float y)
{
	XMFLOAT2 newMousePos = { x, y };

	XMFLOAT2 mouseDist; 
	mouseDist.x = newMousePos.x - mousePos.x;
	mouseDist.y = newMousePos.y - mousePos.y;

	XMVECTOR len = XMVector2Length(XMLoadFloat2(&mouseDist));

	//Makes sure that the camera does not jump when the newMousePos
	//is far away from the oldMousePos
	if (XMVectorGetX(len) > 50.0f) 
	{
		mousePos = newMousePos;
	}
	else 
	{
		//Get the perpendicular axis to the up- and viewDir-axis
		XMVECTOR xAxis = XMVector3Cross(XMLoadFloat4(&this->cameraDir), XMLoadFloat4(&this->cameraUp));

		XMMATRIX rotatorX = XMMatrixRotationAxis(XMLoadFloat4(&this->cameraUp), -mouseDist.x * rotSpeed);
		XMMATRIX rotatorY = XMMatrixRotationAxis(xAxis, -mouseDist.y * rotSpeed);
		XMMATRIX rotator = rotatorX * rotatorY;

		cameraDir.x = XMVectorGetX(XMVector4Transform(XMLoadFloat4(&cameraDir), rotator));
		cameraDir.y = XMVectorGetY(XMVector4Transform(XMLoadFloat4(&cameraDir), rotator));
		cameraDir.z = XMVectorGetZ(XMVector4Transform(XMLoadFloat4(&cameraDir), rotator));

		mousePos = newMousePos;

		UpdateViewMatrix();
	}
}

void Camera::MouseMovement()
{
	POINT cursorPos;
	float x = 0;
	float y = 0;

	if (GetKeyState(VK_LBUTTON) < 0) //Runs if the left mousebutton is pressed
	{ 
		GetCursorPos(&cursorPos);
		x = cursorPos.x;
		y = cursorPos.y;
		MouseUpdate(x, y);
	}
}

void Camera::KeyMovement()
{
	if (GetKeyState('W') & 0x8000) 
	{
		MoveForward();
	}
	if (GetKeyState('S') & 0x8000)
	{
		MoveBackward();
	}
	if (GetKeyState('D') & 0x8000) 
	{
		MoveRight();
	}
	if (GetKeyState('A') & 0x8000) 
	{
		MoveLeft();
	}
	if (GetKeyState('E') & 0x8000) 
	{
		MoveUp();
	}
	if (GetKeyState('Q') & 0x8000) 
	{
		MoveDown();
	}
}

double Camera::ClockToMilliseconds(clock_t ticks)
{
	// units/(units/time) => time (seconds) * 1000 = milliseconds
	return (ticks / (double)CLOCKS_PER_SEC) * 1000.0;
}

void Camera::BeginFrame()
{
	this->beginFrame = clock();
}

void Camera::EndFrame()
{
	this->endFrame = clock();

	this->deltaTime += this->endFrame - this->beginFrame;
	this->frames++;

	this->time = this->endFrame - this->beginFrame;
	
	this->accumulatedTime += this->time;

	//if you really want FPS
	if (ClockToMilliseconds(this->deltaTime) > 1000.0) //every second
	{
		this->frameRate = (double)this->frames * 0.5 + this->frameRate * 0.5; //more stable
		this->frames = 0;
		this->deltaTime -= CLOCKS_PER_SEC;
		this->averageFrameTimeMilliseconds = 1000.0 / (this->frameRate == 0 ? 0.001 : this->frameRate); //per milisecond
		this->fps = "FPS: " + std::to_string((int)this->frameRate) + "\n";
	}
}

std::string Camera::GetFPS()
{
	return this->fps;
}

double Camera::GetAccumulatedTime()
{
	return ClockToMilliseconds(this->accumulatedTime);
}

void Camera::ResetAccumulatedTime()
{
	this->accumulatedTime = 0.0;
}
