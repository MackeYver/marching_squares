//
// Vertex shader
float4 vMain(float2 P : POSITION) : SV_POSITION
{
	float4 Ph = {P.x, P.y, 0.0f, 1.0f};
	return Ph;
}


//
// Fragment shader
float4 pMain() : SV_TARGET
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}