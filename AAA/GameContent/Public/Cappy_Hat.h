#pragma once
#include "MonsterPart.h"
#include "Inhalable.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CCappy_Hat final : public CMonsterPart, public IInhalable
{
    GENERATED_BODY(CCappy_Hat)

public:
    struct CAPPY_HAT_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Cappy_Hat";

private:
    CCappy_Hat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCappy_Hat(const CCappy_Hat& Prototype);
    virtual ~CCappy_Hat() = default;

private:
    virtual HRESULT             Initialize_Prototype() override;
    virtual HRESULT             Initialize(void* pArg) override;
    virtual void                Update(_float fTimeDelta) override;
    virtual void                Late_Update(_float fTimeDelta) override;

public:
    virtual void                Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }
    virtual _bool               Can_BeInhaled(const INHALE_QUERY& q) const override;
    virtual void                Be_Captured(CGameObject* pInhaler) override;
    virtual COPY_ABILITY_TYPE   Get_CopyAbility() const override;
    virtual CGameObject*        Get_GameObject() override;
    virtual _float3             Get_SpatPivotOffset() const { return _float3(0.f, 0.f, 0.f); }

    virtual void                On_SpatBegin() override; // AI/컨트롤러/자체콜라이더 끄고 애님 전환
    virtual void                On_SpatEnd() override; // 풀 반환(Set_Active false) 또는 사망FX

    void                        Enable_HurtBox(_bool b);

    _bool                       Is_Capturing() const { return m_bCaptured; }
    _bool                       Is_Detached() const { return m_bDetached; }
    _bool                       Is_Gone() const { return m_bGone; }
    _bool                       Needs_Container() const { return m_bDetached && m_bActive && !m_bGone; }

private:
    virtual HRESULT             Ready_Components() override;
    HRESULT                     Ready_HurtBox();
    void                        Update_CapturePull(_float fTimeDelta);

    void                        Detach();

private:
    CCollider*                  m_pHurtBox = { nullptr };
    CGameObject*                m_pCaptor = { nullptr };

    _float4x4                   m_matDetachParent{};            // detach 후 부모=단위행렬

    _bool                       m_bDetached = { false };
    _bool                       m_bCaptured = { false };
    _bool                       m_bGone = { false };

    _float                      m_fPullSpeed = { 0.f };
    static constexpr _float     s_fPullAccel = { 40.f };


public:
    static CCappy_Hat*          Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void            Free() override;

};

NS_END