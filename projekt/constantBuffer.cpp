#include "constantBuffer.h"

ConstantBuffer::ConstantBuffer()
{
}

ConstantBuffer::~ConstantBuffer()
{
}

float* ConstantBuffer::GetBuffData(int type)
{
	return buffData[type];
}

XMFLOAT4X4* ConstantBuffer::GetWvpMat()
{
	return &this->wvpMat;
}

void ConstantBuffer::SetData(int type, float* data)
{
	buffData[type] = data;
}

void ConstantBuffer::SetWvpMat(XMMATRIX mat)
{
	XMStoreFloat4x4(&this->wvpMat, mat); // store transposed wvp matrix in constant buffer
}
