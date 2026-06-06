#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CShader;

class CMaterialEx final : public CBase
{
private:
    CMaterialEx() = default;
    virtual ~CMaterialEx() = default;

public:
    HRESULT Initialize(const MATERIAL_DATA& data, const _char* pModelFilePath);
    HRESULT Bind_ShaderResource(CShader* pShader, const _char* pConstantName, MTEX_TYPE eTexType, _uint iIndex);

private:
    CGameInstance_Proxy*    m_pProxy = { nullptr };
    vector<TEXTURE_HANDLE>  m_MaterialHandles[MTEX_TYPE_MAX];

public:
    static CMaterialEx* Create(const MATERIAL_DATA& data, const _char* pModelFilePath);

private:
    virtual void Free() override;
};

NS_END