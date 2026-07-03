#include "Boss.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"

CBoss::CBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBossBase(pDevice, pContext) {
}
CBoss::CBoss(const CBoss& Prototype)
    : CBossBase(Prototype) {
}

HRESULT CBoss::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_TraitFlags = MT_BODYCHECK_DAMAGE;      // MT_INHALABLE 제외 → 완전 비흡입
    m_vHitFlashColor = _float3(0.4f, 0.4f, 0.4f);

    return S_OK;
}

void CBoss::Update_AI(_float fTimeDelta)
{
    if (m_bPhaseTransition && m_eLife != EBOSS_LIFE::DEAD)  
    {
        if (Is_PhaseTransition_Finished())
            m_bPhaseTransition = false;
        return;
    }
    m_bPhaseTransition = false;

    __super::Update_AI(fTimeDelta);           // CBossBase 라이프사이클(+ACTIVE면 Brain Decide)

    if (m_eLife == EBOSS_LIFE::ACTIVE)
        Check_PhaseTransition();
}

void CBoss::Check_PhaseTransition()
{
    const vector<_float>& vThresholds = Get_PhaseThresholds();

    _int   iTarget = 0;
    _float fRatio = Get_HPRatio();
    for (size_t i = 0; i < vThresholds.size(); ++i)
        if (fRatio <= vThresholds[i])
            iTarget = static_cast<_int>(i) + 1;

    if (iTarget > m_iPhase)                    // 페이즈는 전진만(되돌아가지 않음)
    {
        _int iOld = m_iPhase;
        m_iPhase = iTarget;
        m_bPhaseTransition = true;
        Play_PhaseTransition(m_iPhase);
        On_PhaseChanged(iOld, m_iPhase);
    }
}

void CBoss::On_Enter_Corpse()
{
    Publish_Boss_Died();                       // 흡입화 안 함(트레잇 변경 X)
}

void CBoss::Publish_Boss_Died()
{
    m_pGameInstance_Proxy->Publish(EventTag::Boss_Died, nullptr);
}

void CBoss::Free()
{
    __super::Free();
}