#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "external/directx/d3dx12.h"

class Renderer
{
public:
    static Renderer& Get();

    void BeginFrame();
    void EndFrame();

    void ClearRenderTarget(const float clearColor[4]);

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return m_CommandList; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const;
    uint32_t GetCurrentBackBufferIndex() const { return m_CurrentBackBufferIndex; }

private:
    Renderer() = default;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    uint32_t m_CurrentBackBufferIndex = 0;
};