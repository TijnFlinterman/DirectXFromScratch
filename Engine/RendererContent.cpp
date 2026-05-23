#include "RendererContent.h"
#include "Application.h"
#include "Helpers.h"

void RendererContent::UploadBufferResource(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12Resource>* pDestinationResource,
    Microsoft::WRL::ComPtr<ID3D12Resource>* pIntermediateResource,
    size_t numElements, size_t elementSize, const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetDevice().GetDevice();
    size_t bufferSize = numElements * elementSize;

    auto defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto defaultBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &defaultBufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(pDestinationResource->GetAddressOf())));

    if (bufferData)
    {
        auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource->GetAddressOf())));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = bufferSize;

        UpdateSubresources(commandList.Get(),
            pDestinationResource->Get(),
            pIntermediateResource->Get(),
            0, 0, 1, &subresourceData);
    }
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> RendererContent::CreateEmptyRootSignature()
{
    auto device = Application::Get().GetDevice().GetDevice();

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
        &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    D3D12_ROOT_SIGNATURE_FLAGS flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(0, nullptr, 0, nullptr, flags);

    Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    ThrowIfFailed(device->CreateRootSignature(0,
        rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)));

    return rootSignature;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> RendererContent::CreateConstantsRootSignature(
    uint32_t num32BitConstants, uint32_t shaderRegister)
{
    auto device = Application::Get().GetDevice().GetDevice();

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
        &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    D3D12_ROOT_SIGNATURE_FLAGS flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(num32BitConstants, shaderRegister, 0,
        D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, flags);

    Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    ThrowIfFailed(device->CreateRootSignature(0,
        rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)));

    return rootSignature;
}