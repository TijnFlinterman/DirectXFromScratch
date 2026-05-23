#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <chrono>

#if defined(max)
#undef max
#endif

class CommandQueue
{
public:
    void Initialize(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
    void Shutdown();

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList(uint32_t frameIndex);
    void ExecuteCommandList();

    uint64_t Signal();
    bool IsFenceComplete(uint64_t fenceValue);
    void WaitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds(UINT64_MAX));
    void Flush();

    uint64_t SignalFrameFence(uint32_t frameIndex);
    void WaitForFrameFence(uint32_t frameIndex);

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetQueue() const { return m_CommandQueue; }

private:
    static const uint32_t k_NumFrames = 3;

    Microsoft::WRL::ComPtr<ID3D12Device2>            m_Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>        m_CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_CommandAllocators[k_NumFrames];
    Microsoft::WRL::ComPtr<ID3D12Fence>               m_Fence;
    HANDLE                                            m_FenceEvent = nullptr;
    uint64_t                                          m_FenceValue = 0;
    uint64_t                                          m_FrameFenceValues[k_NumFrames] = {};
    D3D12_COMMAND_LIST_TYPE                           m_Type;
};