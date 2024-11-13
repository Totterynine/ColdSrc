struct VS_Output
{
	float4 Pos : SV_POSITION;
    float3 Color : COLOR0;
};

VS_Output main(uint VertexIndex : SV_VertexID)
{
    VS_Output o = {0.0f.xxxx, 0.0f.xxx};

    //const array of positions for the triangle
	const float3 positions[3] = {
		float3(1.f,1.f, 0.0f),
		float3(-1.f,1.f, 0.0f),
		float3(0.f,-1.f, 0.0f)
    };

	//const array of colors for the triangle
	const float3 colors[3] = {
		float3(1.0f, 0.0f, 0.0f), //red
		float3(0.0f, 1.0f, 0.0f), //green
		float3(00.f, 0.0f, 1.0f)  //blue
    };

    o.Pos = float4(positions[VertexIndex], 1.0f);
    o.Color = colors[VertexIndex];

    return o;
}