#include "TriangleDemo.h"

#include "../Application.h"
#include "../Renderer.h"
#include "../RendererContent.h"
#include "../Helpers.h"

#include <d3dcompiler.h>

static float g_TriangleVertices[3][3] = {
    {  0.0f,  0.5f, 0.0f },
    {  0.5f, -0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f },
};

static uint16_t g_TriangleIndices[3] = { 0, 1, 2 };

bool TriangleDemo::LoadContent()
{
    Application& app = Application::Get();
    auto commandList = app.GetCommandQueue().GetCommandList(0);

    RendererContent::UploadBufferResource(commandList,
        &m_VertexBuffer, &m_IntermediateVertexBuffer,
        3, sizeof(Vertex), g_TriangleVertices);

    RendererContent::UploadBufferResource(commandList,
        &m_IndexBuffer, &m_IntermediateIndexBuffer,
        3, sizeof(uint16_t), g_TriangleIndices);

    app.GetCommandQueue().ExecuteCommandList();
    app.GetCommandQueue().Flush();

    m_RootSignature = RendererContent::CreateEmptyRootSignature();
    CreatePipelineState();

    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
        static_cast<float>(app.GetClientWidth()),
        static_cast<float>(app.GetClientHeight()));
    m_ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);

    return true;
}

void TriangleDemo::UnloadContent()
{
    m_VertexBuffer.Reset();
    m_IntermediateVertexBuffer.Reset();
    m_RootSignature.Reset();
    m_PipelineState.Reset();
    m_IndexBuffer.Reset();
    m_IntermediateIndexBuffer.Reset();
}

void TriangleDemo::OnUpdate(float deltaTime)
{
    
}

void TriangleDemo::OnRender()
{
    Renderer& renderer = Renderer::Get();
    Application& app = Application::Get();

    renderer.BeginFrame();

    auto commandList = renderer.GetCommandList();

    // Clear
    float clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
    renderer.ClearRenderTarget(clearColor);

    // Set pipeline
    commandList->SetPipelineState(m_PipelineState.Get());
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());

    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = 3 * sizeof(Vertex);
    m_VertexBufferView.StrideInBytes = sizeof(Vertex);

    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.SizeInBytes = 3 * sizeof(uint16_t);
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;

    // IA
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    // RS
    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    // OM
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = renderer.GetCurrentRTV();
    commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

    renderer.EndFrame();
}

void TriangleDemo::CreatePipelineState()
{
    Application& app = Application::Get();

    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));
    ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
          D3D12_APPEND_ALIGNED_ELEMENT,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = app.GetSwapChain().GetFormat();

    pipelineStateStream.pRootSignature = m_RootSignature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC psoDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(app.GetDevice().GetDevice()->CreatePipelineState(
        &psoDesc, IID_PPV_ARGS(&m_PipelineState)));
}