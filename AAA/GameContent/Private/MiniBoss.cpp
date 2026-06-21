#include "MiniBoss.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Monster_State.h"
#include "Monster_State_Captured.h"

CMiniBoss::CMiniBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster(pDevice, pContext) {
}
CMiniBoss::CMiniBoss(const CMiniBoss& Prototype)
    : CMonster(Prototype) {
}

HRESULT CMiniBoss::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))   
        return E_FAIL;

    m_eLife = EMINIBOSS_LIFE::HIDDEN;
    Set_Active(false);              // 트리거 전까지 비활성 (없으면 m_bActive=false)

    if (const _tchar* pTag = Get_AppearEventTag())
        Subscribe_Event(pTag, [this](void*) { Appear(); });

    m_pCaptureState = CMonster_State_Captured::Create();
    if (m_pCaptureState == nullptr)
        return E_FAIL;

    m_pCaptureState->Set_Owner(this);

    m_TraitFlags = MT_NONE;

    return S_OK;
}

void CMiniBoss::Appear()
{
    if (m_eLife != EMINIBOSS_LIFE::HIDDEN)
        return;

    Set_Active(true);
    Set_Target(Find_Player());
    m_eLife = EMINIBOSS_LIFE::INTRO;
}

void CMiniBoss::Die()
{
    if (m_eLife == EMINIBOSS_LIFE::DEAD) return;
    m_eLife = EMINIBOSS_LIFE::DEAD;
}

void CMiniBoss::Be_Captured(CGameObject* pInhaler)
{
    m_pCaptor = pInhaler;
    m_pCaptureState->Enter();     // CCT off + 초기화 (상태 내부)
    m_eLife = EMINIBOSS_LIFE::EATEN;
}

void CMiniBoss::Update_AI(_float fTimeDelta)
{
    switch (m_eLife)
    {
        case EMINIBOSS_LIFE::HIDDEN:
            return;                                    
        case EMINIBOSS_LIFE::INTRO:
            if (!m_bIntroStarted) 
            { 
                Play_Intro();
                m_bIntroStarted = true; 
            }
            if (Is_Intro_Finished())
            {
                Publish_Boss_Appeared();
                m_eLife = EMINIBOSS_LIFE::ACTIVE;
            }
            if (m_pMovement && !m_pGameInstance_Proxy->Is_EditMode())
                m_pMovement->Move(XMVectorZero(), fTimeDelta);
            return;
        case EMINIBOSS_LIFE::ACTIVE:
            __super::Update_AI(fTimeDelta);          
            break;
        case EMINIBOSS_LIFE::DEAD:
            if (!m_bDeathStarted) { Play_Death(); m_bDeathStarted = true; } 

            if (!m_bCorpse && Is_Death_Finished())    
            {
                Play_DeathLoop();                  
                m_bCorpse = true;
                m_TraitFlags = MT_INHALABLE | MT_STRONG_INHALE_ONLY;
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

        case EMINIBOSS_LIFE::EATEN:
            m_pCaptureState->Update(fTimeDelta); 
            return;
    }
}

void CMiniBoss::Free()
{
    Safe_Release(m_pCaptureState);
    __super::Free();
}