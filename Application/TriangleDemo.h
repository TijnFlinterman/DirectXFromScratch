#pragma once
#include "../IDemo.h"
#include <d3d12.h>
#include <wrl.h>
#include "../external/directx/d3dx12.h"

class TriangleDemo : public IDemo
{
public:
	bool LoadContent() override;
	void UnloadContent() override;
	void OnUpdate(float deltaTime) override;
	void OnRender() override;
	const wchar_t* GetName() const override { return L"Triangle Demo"; }

private:
	void CreatePipelineState();

	struct Vertex
	{
		float Position[3];
	};

	Microsoft::WRL::ComPtr<ID3D12Resource>      m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>      m_IntermediateVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW                    m_VertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	D3D12_VIEWPORT                              m_Viewport;
	D3D12_RECT                                  m_ScissorRect;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IntermediateIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW                m_IndexBufferView;
};