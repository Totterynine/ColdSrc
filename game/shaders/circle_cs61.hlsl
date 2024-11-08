RWTexture2D<float> InTexture : register( u0 );

float sdCircle( in float2 p, in float r ) 
{
    return length(p)-r;
}

[numthreads( 16, 16, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID,
uint3 GTid : SV_GroupThreadID, uint Gidx : SV_GroupIndex )
{
    uint2 inDims;
	InTexture.GetDimensions(inDims.x, inDims.y );

    float2 screenPos = float2(2.0f * DTid.xy - inDims) / inDims.y;
    float circle = sdCircle(screenPos, 0.5f);

    float4 col = InTexture[DTid.xy];
    if(circle < 0.0f)
        col = float4(0.65f, 0.85f, 1.0f, 1.0f);

    InTexture[DTid.xy] = col;

    return;
}