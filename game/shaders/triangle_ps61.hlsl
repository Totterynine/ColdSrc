struct PS_Input
{
    float3 Color : COLOR0;
};

float4 main(PS_Input i) : SV_Target
{
    return float4(i.Color, 1.0f);
}