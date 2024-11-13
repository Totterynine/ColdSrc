RWTexture2D<float4> InTexture : register( u0 );

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
    {
        col = float4(0.65f, 0.85f, 1.0f, 1.0f);
        col *= 1.0 - exp(-6.0*abs(circle));
        col *= 0.8 + 0.2*cos(150.0*circle);
        col = lerp( col, 1.0f.xxxx, 1.0-smoothstep(0.0,0.01,abs(circle)) );
    }

    InTexture[DTid.xy] = col;

    return;
}