#include "UI_BossStatus.h"
#include "UI_GaugeFill.h"
#include "UI_GaugeBarCom.h"
#include "GameInstance.h"

namespace
{
    constexpr const _tchar* COM_GAUGEBAR = TEXT("Com_GaugeBar");
    constexpr const _tchar* PART_HP_FILL = TEXT("Guage");   // 레벨데이터의 보스 게이지 파트 태그
}

CUI_BossStatus::CUI_BossStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject(pDevice, pContext)
{
}

CUI_BossStatus::CUI_BossStatus(const CUI_BossStatus& Prototype)
    : CUIContainerObject(Prototype)
    , m_fDefaultMaxHP{ Prototype.m_fDefaultMaxHP }
    , m_fDefaultCurrHP{ Prototype.m_fDefaultCurrHP }
{
}

HRESULT CUI_BossStatus::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_BossStatus::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CUI_BossStatus::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Priority_Update(fTimeDelta);
}

void CUI_BossStatus::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    if (m_pGaugeBar && !m_pGaugeBar->Is_Bound())
        Try_BindGauge();

#ifdef _DEBUG
    if (m_pGaugeBar)
    {
        if (m_pGameInstance_Proxy->Key_Down(DIK_N)) m_pGaugeBar->Add_Value(-10.f);
        if (m_pGameInstance_Proxy->Key_Down(DIK_M)) m_pGaugeBar->Add_Value(+10.f);
    }
#endif

    __super::Update(fTimeDelta);
}

void CUI_BossStatus::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_BossStatus::Ready_Events()
{
    if (FAILED(__super::Ready_Events()))
        return E_FAIL;

    Subscribe_Event(EventTag::Boss_HP_Updated,
        [this](void* pData)
        {
            auto* p = static_cast<BOSS_HP_UPDATED*>(pData);
            if (nullptr == p) return;

            if (m_pGaugeBar && m_pGaugeBar->Is_Bound())
                m_pGaugeBar->Set_Value(p->fCurrHp, p->fMaxHP);
            else
            {
                m_bPendingHP = true;
                m_fPendingMax = p->fMaxHP;
                m_fPendingCurr = p->fCurrHp;
            }
        });

    Subscribe_Event(EventTag::Boss_Appeared, [this](void*) { Set_Active(true);  });
    Subscribe_Event(EventTag::Boss_Died, [this](void*) { Set_Active(false); });

    return S_OK;
}

void CUI_BossStatus::Try_BindGauge()
{
    auto it = m_UIPartObjects.find(PART_HP_FILL);
    if (it == m_UIPartObjects.end())
        return;

    m_pGaugeBar->Bind_Gauge(static_cast<CUI_GaugeFill*>(it->second));

    if (m_bPendingHP)                    
    {
        m_pGaugeBar->Set_Value(m_fPendingCurr, m_fPendingMax);
        m_bPendingHP = false;
    }
}

HRESULT CUI_BossStatus::Ready_Components()
{
    m_pGaugeBar = Add_Component<CUI_GaugeBarCom>(
        COM_GAUGEBAR, CUI_GaugeBarCom::Create(m_pDevice, m_pContext));
    if (!m_pGaugeBar)
        return E_FAIL;

    return S_OK;
}

CUI_BossStatus* CUI_BossStatus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_BossStatus* pInstance = new CUI_BossStatus(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_BossStatus");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_BossStatus::Clone(void* pArg)
{
    CUI_BossStatus* pInstance = new CUI_BossStatus(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_BossStatus");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_BossStatus::Free()
{
    __super::Free();
}