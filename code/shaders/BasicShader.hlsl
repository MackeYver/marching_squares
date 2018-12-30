cbuffer ConstantBuffer : register(b0)
{
    float4x4 TransformMatrix;
	float4 Colour;
	float3 Padding;
	float Z;
};

//
// Vertex shader
float4 vMain(float2 P : POSITION) : SV_POSITION
{
	float4 Ph = {P.x, P.y, Z, 1.0f};
	return mul(TransformMatrix, Ph);
}


//
// Fragment shader
float4 pMain() : SV_TARGET
{
	return Colour;
}