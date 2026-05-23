#include "CommandQueue.h"
#include "Helpers.h"
#include <cassert>

void CommandQueue::Initialize(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
{
    m_Device = device;
    m_Type = type;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;
    ThrowIfFailed(m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));

    for (uint32_t i = 0; i < k_NumFrames; ++i)
        ThrowIfFailed(m_Device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_CommandAllocators[i])));

    ThrowIfFailed(m_Device->CreateCommandList(0, type,
        m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
    ThrowIfFailed(m_CommandList->Close());

    ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

    m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_FenceEvent && "Failed to create fence event.");
}

void CommandQueue::Shutdown()
{
    Flush();
    ::CloseHandle(m_FenceEvent);
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandQueue::GetCommandList(uint32_t frameIndex)
{
    ThrowIfFailed(m_CommandAllocators[frameIndex]->Reset());
    ThrowIfFailed(m_CommandList->Reset(m_CommandAllocators[frameIndex].Get(), nullptr));
    return m_CommandList;
}

void CommandQueue::ExecuteCommandList()
{
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* const commandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
}

uint64_t CommandQueue::Signal()
{
    uint64_t fenceValueForSignal = ++m_FenceValue;
    ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), fenceValueForSignal));
    return fenceValueForSignal;
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
    return m_Fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::WaitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration)
{
    if (!IsFenceComplete(fenceValue))
    {
        ThrowIfFailed(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent));
        ::WaitForSingleObject(m_FenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void CommandQueue::Flush()
{
    WaitForFenceValue(Signal());
}

uint64_t CommandQueue::SignalFrameFence(uint32_t frameIndex)
{
    m_FrameFenceValues[frameIndex] = Signal();
    return m_FrameFenceValues[frameIndex];
}

void CommandQueue::WaitForFrameFence(uint32_t frameIndex)
{
    WaitForFenceValue(m_FrameFenceValues[frameIndex]);
}