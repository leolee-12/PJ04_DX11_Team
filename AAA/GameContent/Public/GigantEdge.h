#pragma once
#include "MiniBoss.h"

NS_BEGIN(Client)
class CGigantEdge_Body;
class CGigantEdge_Sword;
class CGigantEdge_Shield;

class CGigantEdge final : public CMiniBoss
{
    GENERATED_BODY(CGigantEdge)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge";
    static constexpr _float s_fCCT_Radius = 2.f;
    static constexpr _float s_fCCT_Height = 2.f;

private:
    CGigantEdge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigantEdge(const CGigantEdge& Prototype);
    virtual ~CGigantEdge() = default;

public:
    virtual HRESULT Initialize_Prototype() override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
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

    void  On_Hit(_fvector vAttackerPos, _float fDamage);

protected:
    virtual HRESULT        Ready_Parts() override;
    virtual CMonsterBrain* Create_Brain() override;
    virtual void           Play_Intro() override;
    virtual _bool          Is_Intro_Finished() const override;
    virtual const _tchar* Get_AppearEventTag() const override { return TEXT("GigantEdge_Appear"); }

    virtual _float Get_CapsuleRadius() const override { return s_fCCT_Radius; }
    virtual _float Get_CapsuleHeight() const override { return s_fCCT_Height; }
    virtual void   Play_StateAnimation(MONSTER_STATE_TYPE) override {}

private:
    CGigantEdge_Body* m_pBody = { nullptr };
    CGigantEdge_Sword* m_pSword = { nullptr };
    CGigantEdge_Shield* m_pShield = { nullptr };

    _bool m_bGuarding = { false };
    _bool m_bGroggyRequested = { false };

public:
    static CGigantEdge* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGigantEdge* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END