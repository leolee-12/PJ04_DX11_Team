#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Client)

enum class EBOSS_LIFE { HIDDEN, INTRO, ACTIVE, DEAD };   // EATEN 제거(미니보스 전용)

class CBossBase abstract : public CMonster
{
    GENERATED_BODY_ABSTRACT(CBossBase)

protected:
    CBossBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBossBase(const CBossBase& Prototype);
    virtual ~CBossBase() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;

public:
    void            Appear();                 // 등장 트리거가 호출
    void            Die();

    EBOSS_LIFE      Get_Life() const { return m_eLife; }
    _bool           Is_Active() const { return m_eLife == EBOSS_LIFE::ACTIVE; }

protected:
    virtual void           Update_AI(_float fTimeDelta) override;   // 라이프사이클 드라이버
    virtual CMonsterBrain* Create_Brain() override = 0;
    virtual _bool          Use_StateMachine() const override { return false; }
    virtual const _tchar* Get_AppearEventTag() const { return nullptr; }

    // 연출 훅
    virtual void   Play_Intro() = 0;
    virtual _bool  Is_Intro_Finished() const = 0;
    virtual void   Play_Death() = 0;
    virtual void   Play_DeathLoop() {}
    virtual _bool  Is_Death_Finished() const = 0;
    virtual _float Get_CorpseLinger() const { return 5.f; }

    // ★ 분기 훅: 시체 진입 시 (미니보스=흡입화 / 보스=폭발 등)
    virtual void   On_Enter_Corpse() {}

    // 피격/사망 (기존 CMiniBoss:: 구현을 그대로 이식)
    virtual void   On_Damaged(const ATTACK_INFO& tInfo) override;
    virtual void   On_Death(const ATTACK_INFO& tInfo) override;
    virtual void   On_Hit_Reaction(const ATTACK_INFO& tInfo) {}
    virtual void   On_Death_Reaction(const ATTACK_INFO& tInfo) {}

    CGameObject* Find_Player() const;
    void         Publish_Boss_Appeared();
    void         Publish_HP();

    _float       Get_HPRatio() const { return m_fMaxHP > 0.f ? m_fCurHP / m_fMaxHP : 0.f; }

protected:
    EBOSS_LIFE m_eLife = { EBOSS_LIFE::HIDDEN };
    _bool      m_bIntroStarted = { false };
    _bool      m_bDeathStarted = { false };
    _bool      m_bCorpse = { false };
    _float     m_fCorpseTimer = { 0.f };
    _wstring   m_strBossName = { L"Boss" };

protected:
    virtual void Free() override;
};

NS_END