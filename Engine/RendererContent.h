#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "external/directx/d3dx12.h"
#include <cstdint>

class RendererContent
{
public:
    // Upload a buffer to the GPU, returns the destination resource
    static void UploadBufferResource(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Resource>* pDestinationResource,
        Microsoft::WRL::ComPtr<ID3D12Resource>* pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    // Create a simple root signature with no parameters
    static Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateEmptyRootSignature();

    // Create a root signature with a single 32-bit constants parameter
    static Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateConstantsRootSignature(
        uint32_t num32BitConstants, uint32_t shaderRegister = 0);
};