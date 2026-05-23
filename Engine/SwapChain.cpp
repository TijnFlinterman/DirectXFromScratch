#include "SwapChain.h"
#include "Helpers.h"
#include "external/directx/d3dx12.h"

void SwapChain::Initialize(Microsoft::WRL::ComPtr<ID3D12Device2> device,
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
    HWND hWnd, uint32_t width, uint32_t height)
{
    m_Device = device;
    m_Width = width;
    m_Height = height;
    m_TearingSupported = CheckTearingSupport();

    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = k_NumFrames;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = m_TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1));

    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain1.As(&m_SwapChain));

    // Create RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = k_NumFrames;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap)));
    m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    UpdateRenderTargetViews();
}

void SwapChain::UpdateRenderTargetViews()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (uint32_t i = 0; i < k_NumFrames; ++i)
    {
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i])));
        m_Device->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(m_RTVDescriptorSize);
    }
}

void SwapChain::Resize(uint32_t width, uint32_t height)
{
    if (m_Width == width && m_Height == height)
        return;

    m_Width = width;
    m_Height = height;

    for (uint32_t i = 0; i < k_NumFrames; ++i)
        m_BackBuffers[i].Reset();

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    ThrowIfFailed(m_SwapChain->GetDesc(&swapChainDesc));
    ThrowIfFailed(m_SwapChain->ResizeBuffers(k_NumFrames, width, height,
        swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

    UpdateRenderTargetViews();
}

uint32_t SwapChain::Present()
{
    UINT syncInterval = m_VSync ? 1 : 0;
    UINT presentFlags = m_TearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(m_SwapChain->Present(syncInterval, presentFlags));
    return m_SwapChain->GetCurrentBackBufferIndex();
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetRTV(uint32_t index) const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        index, m_RTVDescriptorSize);
    return rtvHandle;
}

uint32_t SwapChain::GetCurrentBackBufferIndex() const
{
    return m_SwapChain->GetCurrentBackBufferIndex();
}

bool SwapChain::CheckTearingSupport()
{
    BOOL allowTearing = FALSE;
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }
    return allowTearing == TRUE;
}