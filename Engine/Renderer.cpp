#include "Renderer.h"
#include "Application.h"
#include "Helpers.h"

Renderer& Renderer::Get()
{
    static Renderer instance;
    return instance;
}

void Renderer::BeginFrame()
{
    Application& app = Application::Get();
    m_CurrentBackBufferIndex = app.GetSwapChain().GetCurrentBackBufferIndex();

    m_CommandList = app.GetCommandQueue().GetCommandList(m_CurrentBackBufferIndex);

    // Transition back buffer to render target
    auto backBuffer = app.GetSwapChain().GetBackBuffer(m_CurrentBackBufferIndex);
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer.Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_CommandList->ResourceBarrier(1, &barrier);
}

void Renderer::EndFrame()
{
    Application& app = Application::Get();

    // Transition back buffer to present
    auto backBuffer = app.GetSwapChain().GetBackBuffer(m_CurrentBackBufferIndex);
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    m_CommandList->ResourceBarrier(1, &barrier);

    app.GetCommandQueue().ExecuteCommandList();
    app.GetCommandQueue().SignalFrameFence(m_CurrentBackBufferIndex);

    m_CurrentBackBufferIndex = app.GetSwapChain().Present();

    app.GetCommandQueue().WaitForFrameFence(m_CurrentBackBufferIndex);
}

void Renderer::ClearRenderTarget(const float clearColor[4])
{
    Application& app = Application::Get();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = app.GetSwapChain().GetRTV(m_CurrentBackBufferIndex);
    m_CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::GetCurrentRTV() const
{
    return Application::Get().GetSwapChain().GetRTV(m_CurrentBackBufferIndex);
}