#pragma once
#include <DirectXMath.h>
#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <string>

using namespace DirectX;

class Camera
{
public:
	Camera(unsigned int width, unsigned int height);
	~Camera();

	XMFLOAT4X4* GetCamProjMat();
	XMFLOAT4X4* GetCamViewMat();

	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void MoveUp();
	void MoveDown();
	void UpdateViewMatrix();
	void MouseUpdate(float x, float y);
	void MouseMovement();
	void KeyMovement();

	double ClockToMilliseconds(clock_t ticks);
	void BeginFrame();
	void EndFrame();
	std::string GetFPS();
	double GetAccumulatedTime();
	void ResetAccumulatedTime();

private:
	float moveSpeed;
	float rotSpeed;
	XMFLOAT2 mousePos;

	XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
	XMFLOAT4X4 cameraViewMat; // this will store our view matrix

	XMFLOAT4 cameraPosition; // this is our cameras position vector
	XMFLOAT4 cameraDir;
	XMFLOAT4 cameraUp; // the worlds up vector

	clock_t deltaTime;
	clock_t beginFrame;
	clock_t endFrame;
	unsigned int frames;
	double frameRate;
	double averageFrameTimeMilliseconds;
	double time;
	double accumulatedTime;
	std::string fps;
};