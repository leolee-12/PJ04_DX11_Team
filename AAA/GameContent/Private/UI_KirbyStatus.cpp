#include "UI_KirbyStatus.h"
#include "UI_GaugeFill.h"
#include "UI_Image.h"
#include "UI_GaugeBarCom.h"
#include "GameInstance.h"

namespace
{
    constexpr const _tchar* COM_GAUGEBAR = TEXT("Com_GaugeBar");

    constexpr const _tchar* PART_HP_FILL_CENTER = TEXT("Guage");
}

CUI_KirbyStatus::CUI_KirbyStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject(pDevice, pContext)
{
}

CUI_KirbyStatus::CUI_KirbyStatus(const CUI_KirbyStatus& Prototype)
    : CUIContainerObject(Prototype)
    , m_fDefaultMaxHP{ Prototype.m_fDefaultMaxHP }
    , m_fDefaultCurrHP{ Prototype.m_fDefaultCurrHP }
{
}

HRESULT CUI_KirbyStatus::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_KirbyStatus::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CUI_KirbyStatus::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Priority_Update(fTimeDelta);
}

void CUI_KirbyStatus::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    if (m_pGaugeBar && !m_pGaugeBar->Is_Bound())   // 파트 로드 후 1회 연결
    {
        auto it = m_UIPartObjects.find(PART_HP_FILL_CENTER);
        if (it != m_UIPartObjects.end())
            m_pGaugeBar->Bind_Gauge(static_cast<CUI_GaugeFill*>(it->second));
    }
    // m_pGaugeBar->Update() 호출 불필요

#ifdef _DEBUG
    if (m_pGaugeBar)
    {
        if (m_pGameInstance_Proxy->Key_Down(DIK_K)) m_pGaugeBar->Add_Value(-10.f);
        if (m_pGameInstance_Proxy->Key_Down(DIK_L)) m_pGaugeBar->Add_Value(+10.f);
    }
#endif

    __super::Update(fTimeDelta);
}

void CUI_KirbyStatus::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_KirbyStatus::Ready_Events()
{
    if (FAILED(__super::Ready_Events()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_KirbyStatus::Ready_Components()
{
    CUI_GaugeBarCom::GAUGEBAR_DESC Desc{};
    Desc.fMaxValue = m_fDefaultMaxHP;
    Desc.fCurrent = m_fDefaultCurrHP;

    m_pGaugeBar = Add_Component<CUI_GaugeBarCom>(
        COM_GAUGEBAR,
        CUI_GaugeBarCom::Create(m_pDevice, m_pContext));

    if (!m_pGaugeBar)
        return E_FAIL;

    return S_OK;
}

CUI_KirbyStatus* CUI_KirbyStatus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_KirbyStatus* pInstance = new CUI_KirbyStatus(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_KirbyStatus");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_KirbyStatus::Clone(void* pArg)
{
    CUI_KirbyStatus* pInstance = new CUI_KirbyStatus(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_KirbyStatus");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_KirbyStatus::Free()
{
    __super::Free();
}