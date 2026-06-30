#pragma once

#include "GameContent_Defines.h"

#include "Effect_MeshParticle.h"   // Effect_Mesh.h -> Effect_MeshParticle.h

NS_BEGIN(Client)

class CRockFloorEffect final : public CEffect_MeshParticle   // CEffect_Mesh -> CEffect_MeshParticle
{
    GENERATED_BODY(CRockFloorEffect)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_RockFloorEffect";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Proto_Model_RockFloorEffect";

private:
    CRockFloorEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRockFloorEffect(const CRockFloorEffect& Prototype);
    virtual ~CRockFloorEffect() = default;

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
    static CRockFloorEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free();
};

NS_END