#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>

class SwapChain
{
public:
    static const uint32_t k_NumFrames = 3;

    void Initialize(Microsoft::WRL::ComPtr<ID3D12Device2> device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
        HWND hWnd, uint32_t width, uint32_t height);

    void Resize(uint32_t width, uint32_t height);
    uint32_t Present();

    Microsoft::WRL::ComPtr<ID3D12Resource>      GetBackBuffer(uint32_t index) const { return m_BackBuffers[index]; }
    D3D12_CPU_DESCRIPTOR_HANDLE                 GetRTV(uint32_t index) const;
    uint32_t                                    GetCurrentBackBufferIndex() const;
    DXGI_FORMAT                                 GetFormat() const { return DXGI_FORMAT_R8G8B8A8_UNORM; }

private:
    bool CheckTearingSupport();
    void UpdateRenderTargetViews();

    Microsoft::WRL::ComPtr<ID3D12Device2>        m_Device;
    Microsoft::WRL::ComPtr<IDXGISwapChain4>      m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D12Resource>       m_BackBuffers[k_NumFrames];
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    UINT                                         m_RTVDescriptorSize = 0;
    bool                                         m_TearingSupported = false;
    bool                                         m_VSync = true;
    uint32_t                                     m_Width = 0;
    uint32_t                                     m_Height = 0;
};