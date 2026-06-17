#include "MiniBoss.h"
#include "GameInstance.h"
#include "Monster_Movement.h"

CMiniBoss::CMiniBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster(pDevice, pContext) {
}
CMiniBoss::CMiniBoss(const CMiniBoss& Prototype)
    : CMonster(Prototype) {
}

HRESULT CMiniBoss::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))   return E_FAIL;

    // 파트 → 이동 → AI(brain) 순서
    if (FAILED(Ready_Parts()))      return E_FAIL;   // 자식 구현
    if (FAILED(Ready_Movement()))   return E_FAIL;   // CMonster
    if (FAILED(Ready_AI()))         return E_FAIL;   // CMonster → Create_Brain()(자식 BT)

    m_eLife = EMINIBOSS_LIFE::HIDDEN;
    Set_Active(false);              // 트리거 전까지 비활성 (없으면 m_bActive=false)

    if (const _tchar* pTag = Get_AppearEventTag())
        Subscribe_Event(pTag, [this](void*) { Appear(); });

    return S_OK;
}

void CMiniBoss::Appear()
{
    if (m_eLife != EMINIBOSS_LIFE::HIDDEN)
        return;

    Set_Active(true);
    m_eLife = EMINIBOSS_LIFE::INTRO;
}

void CMiniBoss::Die()
{
    if (m_eLife == EMINIBOSS_LIFE::DEAD) return;
    m_eLife = EMINIBOSS_LIFE::DEAD;
}

void CMiniBoss::Update_AI(_float fTimeDelta)
{
    switch (m_eLife)
    {
        case EMINIBOSS_LIFE::HIDDEN:
            return;                                       // 트리거 전: AI 정지

        case EMINIBOSS_LIFE::INTRO:
            if (!m_bIntroStarted) 
            { 
                Play_Intro();
                m_bIntroStarted = true; 
            }
            if (Is_Intro_Finished())
                m_eLife = EMINIBOSS_LIFE::ACTIVE;         // ★ 여기서부터 움직임 시작
            // 인트로 중 중력/접지만 유지 (이동 의도 없음)
            if (m_pMovement && !m_pGameInstance_Proxy->Is_EditMode())
                m_pMovement->Move(XMVectorZero(), fTimeDelta);
            return;

        case EMINIBOSS_LIFE::ACTIVE:
            __super::Update_AI(fTimeDelta);               // Perceive + brain->Decide(BT) + Move
            break;

        case EMINIBOSS_LIFE::DEAD:
            if (!m_bDeathStarted) { Play_Death(); m_bDeathStarted = true; }   // DeathStart (브레인 정지)

            if (!m_bCorpse && Is_Death_Finished())     // 쓰러짐 끝 → 시체 루프 + 링거 시작
            {
                Play_DeathLoop();                      // DeathEndWait 루프
                m_bCorpse = true;
            }
            if (m_bCorpse)
            {
                m_fCorpseTimer += fTimeDelta;
                if (m_fCorpseTimer >= Get_CorpseLinger() && m_bActive)
                {
                    if (const _tchar* pTag = Get_AppearEventTag())
                        UnSubscribe_Event(pTag);
                    Enable_Controller(false);   // ← CCT 충돌/쿼리 비활성 (시체 통과 가능)
                    Set_Active(false);
                }   // 5초 뒤 제거(지연 삭제)
            }

            if (m_pMovement && !m_pGameInstance_Proxy->Is_EditMode())
                m_pMovement->Move(XMVectorZero(), fTimeDelta);          // 시체 중력만
            return;
    }
}

void CMiniBoss::Free()
{
    __super::Free();
}