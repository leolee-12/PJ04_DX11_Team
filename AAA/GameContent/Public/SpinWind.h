#pragma once

#include "GameContent_Defines.h"

#include "Effect_MeshParticle.h"

NS_BEGIN(Client)

class CSpinWind final : public CEffect_MeshParticle
{
    GENERATED_BODY(CSpinWind)
    PROPERTY(_float, m_fSpawnHeightMin, L"Spawn Height Min_W", L"SpinWind");
    PROPERTY(_float, m_fSpawnHeightMax, L"Spawn Height Max_W", L"SpinWind");

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_SpinWind";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Proto_Model_SpinWind";

private:
    CSpinWind(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSpinWind(const CSpinWind& Prototype);
    virtual ~CSpinWind() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
    virtual void Effect_Start() override;

public:
    static CSpinWind* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free();
};

NS_END