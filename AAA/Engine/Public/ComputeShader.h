#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CComputeShader final : public CBase
{
private:
    CComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CComputeShader() = default;

public:
    HRESULT Initialize(const _tchar* pShaderFilePath, const _char* pEntryPoint);

public:
    void Bind();                                  // CSSetShader
    void Dispatch(_uint x, _uint y, _uint z);

    void Bind_CBV(_uint iSlot, ID3D11Buffer* pCB);
    void Bind_SRV(_uint iSlot, ID3D11ShaderResourceView* pSRV);
    void Bind_UAV(_uint iSlot, ID3D11UnorderedAccessView* pUAV);

    void Unbind_SRVs(_uint iSlot, _uint iCount);
    void Unbind_UAVs(_uint iSlot, _uint iCount);
    void Unbind_Shader();                         // CSSetShader(null)

private:
    ID3D11Device* m_pDevice = { nullptr };
    ID3D11DeviceContext* m_pContext = { nullptr };
    ID3D11ComputeShader* m_pComputeShader = { nullptr };

public:
    static CComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
        const _tchar* pShaderFilePath, const _char* pEntryPoint);
    virtual void Free() override;
};

NS_END