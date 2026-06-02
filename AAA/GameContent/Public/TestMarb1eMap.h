#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CTestMarb1eMap final : public CGameObject
{
    GENERATED_BODY(CTestMarb1eMap)

    // ---- editor-tunable map shader params ----
    PROPERTY(_float, m_fNormalStrength, L"Normal Strength", L"Map")
    PROPERTY(_float, m_fAOStrength, L"VtxColor AO", L"Map")
    PROPERTY(_bool, m_bTopProjection, L"Top Projection", L"Map")
    PROPERTY(_float2, m_vTopUVScale, L"TopUV Scale", L"Map")
    PROPERTY(_float, m_fTopUVRotate, L"TopUV Rotate", L"Map")
    PROPERTY(_float2, m_vTopUVOffset, L"TopUV Offset", L"Map")
    PROPERTY(_float2, m_vBaseUVScale, L"Base UV Scale", L"Map")

    PROPERTY(_float, m_fMossAmount, L"Moss Amount", L"Map.Moss")

    PROPERTY(_float, m_fDirtAmount, L"Dirt Amount", L"Map.Dirt")

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TestMarb1eMap";

protected:
    CTestMarb1eMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CTestMarb1eMap(const CTestMarb1eMap& Prototype);
    virtual ~CTestMarb1eMap() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };

private:
    virtual HRESULT Ready_Events() override { return S_OK; }
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();

public:
    static CTestMarb1eMap* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free();
};

NS_END