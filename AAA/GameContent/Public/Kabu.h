#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"
#include "RailRideable.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKabu_Body;

class CKabu final : public CMonster, public IRailRideable
{
	GENERATED_BODY(CKabu)

public:
    struct KABU_DESC : public CContainerObject::COTAINEROBJECT_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Kabu";

private:
    CKabu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CKabu(const CKabu& Prototype);
    virtual ~CKabu() = default;

protected:
    virtual HRESULT         Initialize_Prototype() override;
    virtual HRESULT         Initialize(void* pArg) override;

public:
    virtual void            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    virtual _float          Get_CapsuleRadius() const override { return 0.75f; }
    virtual _float          Get_CapsuleHeight() const override { return 0.35f; }
    virtual _float          Get_InteractRadius() const override { return 0.f; }
    virtual _bool           Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

    virtual CAnimator*      Get_BodyAnimator() const override;

    // IRailRideable
    virtual void            Set_RailDesc(const LD_RAIL_DESC& Desc) override;

    _bool                   Is_Visible() const { return m_bVisible; }
    void                    Set_Visible(_bool bVisible);

protected:
    virtual CMonsterBrain*  Create_Brain() override; 
    virtual HRESULT         Create_Movement() override;
    virtual HRESULT         Ready_State() override;
    virtual HRESULT         Ready_AnimEvents() override;

    virtual void			On_Damaged(const ATTACK_INFO& tInfo) override;


private:
    HRESULT                 Ready_PartObjects();
    virtual void            On_Deserialized() override;

private:
    CKabu_Body*             m_pBody = { nullptr };

    _bool                   m_bVisible = { false };         // 워프 중일 때 안보이게 하기 위함

public:
    static CKabu*           Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END