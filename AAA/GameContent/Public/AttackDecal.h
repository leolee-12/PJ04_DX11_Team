#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCullingState;
NS_END

NS_BEGIN(Client)

class CAttackDecal : public CGameObject
{
    GENERATED_BODY(CAttackDecal)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_AttackDecal";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Proto_Model_AttackDecal";

protected:
    CAttackDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CAttackDecal(const CAttackDecal& Prototype);
    virtual ~CAttackDecal() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render_Decal() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

public:
    void  Place(const _float3& vGroundPos, _float fRadius, _float fLifeTime);
    _bool Is_Expired() const { return m_bExpired; }
    void  Set_Alpha(_float fAlpha) { m_fDecalAlpha = fAlpha; }

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CCullingState* m_pCullingState = { nullptr };

    _float3 m_vLocalBoundsCenter = { 0.f, 0.f, 0.f };
    _float3 m_vLocalBoundsExtents = { 0.5f, 0.5f, 0.5f };

    _float  m_fDecalAlpha = { 1.f };
    _float  m_fAge = { 0.f };
    _float  m_fLifeTime = { 1.f };
    _bool   m_bExpired = { false };
    _uint   m_iMaterialID = { WORLD_STATIC_ID };

public:
    static CAttackDecal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END