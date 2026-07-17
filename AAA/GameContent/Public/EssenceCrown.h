#pragma once
#include "GameContent_Defines.h"
#include "Effect_Mesh.h"

NS_BEGIN(Client)

class CEssenceCrown final : public CEffect_Mesh
{
    GENERATED_BODY(CEssenceCrown)

    PROPERTY(_bool, m_bUseRingDeform, L"Use Ring Deform_R", L"Ring");
    PROPERTY(_float, m_fRingHeight, L"Ring Height_R", L"Ring");
    PROPERTY(_float, m_fRingStartRadius, L"Ring Start Radius_R", L"Ring");
    PROPERTY(_float, m_fRingEndRadius, L"Ring End Radius_R", L"Ring");

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_EssenceCrown";

private:
    CEssenceCrown(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEssenceCrown(const CEssenceCrown& Prototype);
    virtual ~CEssenceCrown() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    static CEssenceCrown* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free();
};

NS_END