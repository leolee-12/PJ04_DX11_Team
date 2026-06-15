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
    HRESULT Initialize(const MATERIAL_DATA& data);
    HRESULT Bind_ShaderResource(CShader* pShader, const _char* pConstantName, MTEX_TYPE eTexType, _uint iIndex);
    _uint   Get_TextureCount(MTEX_TYPE eType) const;

private:
    CGameInstance_Proxy*    m_pProxy = { nullptr };
    vector<TEXTURE_HANDLE>  m_MaterialHandles[MTEX_TYPE_MAX];

public:
    static CMaterialEx* Create(const MATERIAL_DATA& data);

private:
    virtual void Free() override;
};

NS_END