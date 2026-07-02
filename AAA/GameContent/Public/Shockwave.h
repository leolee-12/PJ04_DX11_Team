#pragma once

#include "GameContent_Defines.h"

#include "Effect_Mesh.h"

NS_BEGIN(Client)

// 고릴라 발구르기 파동: Donut(링) 메쉬가 커지며 페이드아웃
class CShockwave final : public CEffect_Mesh
{
    GENERATED_BODY(CShockwave)

public:
    struct SHOCKWAVE_DESC : public CEffect_Mesh::EFFECT_MESH_DESC
    {
    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Shockwave_Ring";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Proto_Model_Shockwave";

private:
    CShockwave(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CShockwave(const CShockwave& Prototype);
    virtual ~CShockwave() = default;

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
    static CShockwave* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free();
};

NS_END