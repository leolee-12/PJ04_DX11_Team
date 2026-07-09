#include "BossBase.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "GameContrnt_Events.h"

CBossBase::CBossBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster(pDevice, pContext) {
}
CBossBase::CBossBase(const CBossBase& Prototype)
    : CMonster(Prototype) {
}

HRESULT CBossBase::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_eLife = EBOSS_LIFE::HIDDEN;
    Set_Active(false);   
    Enable_Colliders(false);
    Enable_Controller(false);

    // yse 추가
    if (const _tchar* pTag = Get_AppearEventTag())
    {
        Subscribe_Event(pTag,
            [this](void* pData)
            {
                if (Should_AppearFromEvent(pData))
                    Appear();
            });
    }

    Subscribe_Event(EventTag::Query_Boss, [this](void* p) {
        if (auto q = static_cast<BOSS_QUERY*>(p)) q->pBoss = this;
        });

    // 보스들은 사운드 효과 감쇠 X
    m_bSFX2D = true;

    return S_OK;
}

void CBossBase::Appear()
{
    if (m_eLife != EBOSS_LIFE::HIDDEN)
        return;

    Set_Active(true);
    Enable_Colliders(true);
    Enable_Controller(true);
    Set_Target(Find_Player());
    m_eLife = EBOSS_LIFE::INTRO;
}

void CBossBase::Die()
{
    if (m_eLife == EBOSS_LIFE::DEAD) return;
    m_eLife = EBOSS_LIFE::DEAD;
}

void CBossBase::Damaged(const ATTACK_INFO& tInfo)
{
    if (m_fHitInvuln > 0.f)
        return;                          

    const _float fHPBefore = m_fCurHP;
    __super::Damaged(tInfo);             

    if (m_fCurHP < fHPBefore)            
        m_fHitInvuln = HIT_INVULN_SEC;
}

void CBossBase::Update_AI(_float fTimeDelta)
{
    if (m_fHitInvuln > 0.f)
        m_fHitInvuln = max(0.f, m_fHitInvuln - fTimeDelta);

    switch (m_eLife)
    {
        case EBOSS_LIFE::HIDDEN:
            return;

        case EBOSS_LIFE::INTRO:
            if (!m_bIntroStarted)
            {
                Play_Intro();
                m_bIntroStarted = true;
            }
            if (Is_Intro_Finished())
            {
                Publish_Boss_Appeared();
                m_eLife = EBOSS_LIFE::ACTIVE;
            }
            if (m_pMovement && !m_pGameInstance_Proxy->Is_EditMode())
                m_pMovement->Move(XMVectorZero(), fTimeDelta);
            return;

        case EBOSS_LIFE::ACTIVE:
            __super::Update_AI(fTimeDelta); 
            break;

        case EBOSS_LIFE::DEAD:
            if (!m_bDeathStarted) { Play_Death(); m_bDeathStarted = true; }

            if (!m_bCorpse && Is_Death_Finished())
            {
                Play_DeathLoop();
                m_bCorpse = true;
                On_Enter_Corpse();         
            }
            if (m_bCorpse)
            {
                m_fCorpseTimer += fTimeDelta;
                if (m_fCorpseTimer >= Get_CorpseLinger() && m_bActive)
                {
                    if (const _tchar* pTag = Get_AppearEventTag())
                        UnSubscribe_Event(pTag);
                    Enable_Controller(false);
                    Enable_Colliders(false);
                    Set_Active(false);
                }
            }
            if (m_pMovement && !m_pGameInstance_Proxy->Is_EditMode())
                m_pMovement->Move(XMVectorZero(), fTimeDelta);
            return;
    }
}

void CBossBase::On_Damaged(const ATTACK_INFO& tInfo)
{
    On_Hit_Reaction(tInfo);
    Publish_HP();
}

void CBossBase::On_Death(const ATTACK_INFO& tInfo)
{
    On_Death_Reaction(tInfo);
    Publish_HP();
    Die();
}

CGameObject* CBossBase::Find_Player() const
{
    PLAYER_QUERY q;
    m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &q);
    return q.pPlayer;
}

void CBossBase::Publish_Boss_Appeared()
{
    BOSS_HP_APPEARED desc{};
    desc.strBossName = m_strBossName;
    desc.fMaxHP = m_fMaxHP;
    desc.fCurrHp = m_fCurHP;
    m_pGameInstance_Proxy->Publish(EventTag::Boss_HP_Appeared, &desc);

    _bool bShow = true;
    m_pGameInstance_Proxy->Publish(EventTag::HUD_SetVisible, &bShow);
    m_pGameInstance_Proxy->Publish(EventTag::Letterbox_End, nullptr);
}

void CBossBase::Publish_HP()
{
    BOSS_HP_UPDATED hp{};
    hp.fMaxHP = m_fMaxHP;
    hp.fCurrHp = m_fCurHP;
    m_pGameInstance_Proxy->Publish(EventTag::Boss_HP_Updated, &hp);
}

void CBossBase::Free()
{
    __super::Free();
}