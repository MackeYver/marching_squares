cbuffer ConstantBuffer : register(b0)
{
    float4x4 TransformMatrix;
	float4 Colour;
};

//
// Vertex shader
float4 vMain(float2 P : POSITION) : SV_POSITION
{
	float4 Ph = {P.x, P.y, 0.0f, 1.0f};
	return mul(TransformMatrix, Ph);
}


//
// Fragment shader
float4 pMain() : SV_TARGET
{
	return Colour;
}