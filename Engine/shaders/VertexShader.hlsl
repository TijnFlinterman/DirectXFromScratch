struct VertexInput
{
    float3 Position : POSITION;
};

struct VertexOutput
{
    float4 Position : SV_Position;
};

VertexOutput main(VertexInput IN)
{
    VertexOutput OUT;
    OUT.Position = float4(IN.Position, 1.0f);
    return OUT;
}