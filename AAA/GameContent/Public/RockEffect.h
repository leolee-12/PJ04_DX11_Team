#pragma once

#include "GameContent_Defines.h"

#include "Effect_MeshParticle.h"

NS_BEGIN(Client)

class CRockEffect final : public CEffect_MeshParticle   
{
    GENERATED_BODY(CRockEffect)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_RockEffect";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Proto_Model_RockEffect";

private:
    CRockEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRockEffect(const CRockEffect& Prototype);
    virtual ~CRockEffect() = default;

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
    static CRockEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free();
};

NS_END