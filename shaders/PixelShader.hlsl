Texture2D t1[] : register(t0);
SamplerState s1 : register(s0);
cbuffer texInd : register(b4)
{
	int index;
}

struct VSOut
{
	float4 pos : SV_Position;
	//float4 color : color;
	float2 uv : uv;
};

float4 main(VSOut input) : SV_TARGET0
{
	// texture
	float4 col = t1[index].Sample(s1, input.uv);

	return col;
}