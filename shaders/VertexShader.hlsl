struct VSOut
{
	float4 pos : SV_Position;
	//float4 color : color;
	float2 uv: uv;
};

StructuredBuffer<float3> pos : register(t0);
//StructuredBuffer<float3> col : register(t1);
StructuredBuffer<float2> uv : register(t1);

cbuffer CBmatrix : register(b2)
{
	float4x4 wvp;
}

VSOut main(uint vertexId : SV_VertexID)
{
	VSOut output = (VSOut)0;

	output.pos = mul(float4(pos[vertexId], 1.0), wvp);
	output.uv = uv[vertexId];

	return output;
}
