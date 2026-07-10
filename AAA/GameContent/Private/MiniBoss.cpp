#include "MiniBoss.h"
#include "GameInstance.h"
#include "Monster_State.h"
#include "Monster_State_Captured.h"
#include "GameContrnt_Events.h"

CMiniBoss::CMiniBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBossBase(pDevice, pContext) {
}
CMiniBoss::CMiniBoss(const CMiniBoss& Prototype)
    : CBossBase(Prototype) {
}

HRESULT CMiniBoss::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))    
        return E_FAIL;

    m_pCaptureState = CMonster_State_Captured::Create();
    if (m_pCaptureState == nullptr)
        return E_FAIL;
    m_pCaptureState->Set_Owner(this);

    m_TraitFlags = MT_NONE;                   
    return S_OK;
}

void CMiniBoss::Update_AI(_float fTimeDelta)
{
    if (m_bEaten)
    {
        m_pCaptureState->Update(fTimeDelta);  
        return;
    }
    __super::Update_AI(fTimeDelta);
}

void CMiniBoss::Be_Captured(CGameObject* pInhaler)
{
    m_pCaptor = pInhaler;
    m_pCaptureState->Enter();              
    m_bEaten = true;
}

void CMiniBoss::On_Swallowed()
{
    m_pGameInstance_Proxy->Publish(EventTag::Boss_Died, nullptr);
	__super::On_Swallowed();
}

void CMiniBoss::On_Enter_Corpse()
{
    m_TraitFlags = MT_INHALABLE | MT_STRONG_INHALE_ONLY;  
    m_pGameInstance_Proxy->Publish(EventTag::Boss_Died, nullptr);
}

void CMiniBoss::Free()
{
    Safe_Release(m_pCaptureState);
    __super::Free();
}