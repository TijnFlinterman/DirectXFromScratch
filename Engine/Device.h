#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class Device
{
public:
    void Initialize(bool useWarp = false);

    Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const { return m_Device; }
    Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter() const { return m_Adapter; }

private:
    Microsoft::WRL::ComPtr<IDXGIAdapter4> QueryAdapter(bool useWarp);

    Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> m_Adapter;
};