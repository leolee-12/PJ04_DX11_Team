#pragma once
#include "MiniBoss.h"

NS_BEGIN(Engine)

NS_END

NS_BEGIN(Client)
class CGigantEdge_Body;
class CGigantEdge_Sword;
class CGigantEdge_Shield;

class CGigantEdge final : public CMiniBoss
{
    GENERATED_BODY(CGigantEdge)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge";
    static constexpr _float s_fCCT_Radius = 1.3f;
    static constexpr _float s_fCCT_Height = 0.5f;

private:
    CGigantEdge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigantEdge(const CGigantEdge& Prototype);
    virtual ~CGigantEdge() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }
    virtual void    On_Deserialized() override;

public:
    CGigantEdge_Body* Get_Body()   const { return m_pBody; }
    CGigantEdge_Sword* Get_Sword()  const { return m_pSword; }
    CGigantEdge_Shield* Get_Shield() const { return m_pShield; }

    void  Set_Guarding(_bool b) { m_bGuarding = b; }
    _bool Is_Guarding() const { return m_bGuarding; }
    _bool Is_GroggyRequested() const { return m_bGroggyRequested; }
    void  Clear_Groggy() { m_bGroggyRequested = false; }

protected:
    virtual HRESULT        Ready_Parts() override;
    virtual CMonsterBrain* Create_Brain() override;
    virtual const _tchar*  Get_AppearEventTag() const override { return TEXT("GigantEdge_Appear"); }

    virtual CAnimator*      Get_BodyAnimator() const override;
    virtual void           Play_Intro() override;
    virtual _bool          Is_Intro_Finished() const override;
    virtual void           Play_Death() override;
    virtual void           Play_DeathLoop() override;
    virtual _bool          Is_Death_Finished() const override;

    virtual _float         Get_CapsuleRadius() const override { return s_fCCT_Radius; }
    virtual _float         Get_CapsuleHeight() const override { return s_fCCT_Height; }
    virtual _float		   Get_InteractRadius() const override { return 0.f; }
    virtual _bool		   Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

private:
    CGigantEdge_Body*   m_pBody   = { nullptr };
    CGigantEdge_Sword*  m_pSword  = { nullptr };
    CGigantEdge_Shield* m_pShield = { nullptr };

    _bool m_bGuarding        = { false };
    _bool m_bGroggyRequested = { false };

private:
    virtual _bool Block_Hit(const ATTACK_INFO& tInfo) override;

#ifdef _DEBUG
public:
    _bool Dbg_ForceInRange()  const { return m_bDbgInRange; }
    _bool Dbg_WalkInPlace()   const { return m_bDbgWalkInPlace; }
    _int  Dbg_ForceAttack()   const { return m_iDbgAttack; }

private:
    void  Debug_KeyInput();
    _bool m_bDbgInRange     = { false };
    _bool m_bDbgWalkInPlace = { false };
    _int  m_iDbgAttack      = { -1 };
    _bool m_bDbgSwordDrawn  = { false };
#endif

public:
    static CGigantEdge* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGigantEdge* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END