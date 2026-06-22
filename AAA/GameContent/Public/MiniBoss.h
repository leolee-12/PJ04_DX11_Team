#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Client)

class CMonster_State;

enum class EMINIBOSS_LIFE { HIDDEN, INTRO, ACTIVE, DEAD, EATEN };

class CMiniBoss abstract : public CMonster
{
    GENERATED_BODY_ABSTRACT(CMiniBoss)

protected:
    CMiniBoss(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
    CMiniBoss(const CMiniBoss& Prototype);
    virtual ~CMiniBoss() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;

public:
    void            Appear();                              // 룸 트리거가 호출
    void            Die();

    EMINIBOSS_LIFE  Get_Life() const { return m_eLife; }
    _bool           Is_Active() const { return m_eLife == EMINIBOSS_LIFE::ACTIVE; }

public:
    virtual void				Be_Captured(CGameObject* pInhaler) override;

    virtual _bool               Is_Touch_Harmful() const override
    {
        return m_eLife == EMINIBOSS_LIFE::ACTIVE;
    }

protected:
    EMINIBOSS_LIFE m_eLife = { EMINIBOSS_LIFE::HIDDEN };
    _bool          m_bIntroStarted = { false };
    _bool          m_bDeathStarted = { false };    
    _bool          m_bCorpse = { false };   
    _float         m_fCorpseTimer = { 0.f };

    CMonster_State* m_pCaptureState = { nullptr };

    _wstring        m_strBossName = { L"Boss" };

protected:
    virtual void    Update_AI(_float fTimeDelta) override; // 라이프사이클 게이팅

    virtual CMonsterBrain* Create_Brain() override = 0;
    virtual _bool          Use_StateMachine() const override { return false; }
    virtual const _tchar*  Get_AppearEventTag() const { return nullptr; }

    virtual void   Play_Intro() = 0;
    virtual _bool  Is_Intro_Finished() const = 0;
    virtual void   Play_Death() = 0;
    virtual void   Play_DeathLoop() {}
    virtual _bool  Is_Death_Finished() const = 0;
    virtual _float Get_CorpseLinger() const { return 5.f; }

    virtual void   On_Damaged(const ATTACK_INFO& tInfo) override;
    virtual void   On_Death(const ATTACK_INFO& tInfo) override;   
    virtual void   On_Hit_Reaction(const ATTACK_INFO& tInfo) {}
    virtual void   On_Death_Reaction(const ATTACK_INFO& tInfo) {}

    CGameObject*   Find_Player() const;
    void           Publish_Boss_Appeared();
    void           Publish_HP();

protected:
    virtual void Free() override;
};

NS_END