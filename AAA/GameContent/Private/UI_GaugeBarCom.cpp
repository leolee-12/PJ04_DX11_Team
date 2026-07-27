#include "UI_GaugeBarCom.h"
#include "UI_GaugeFill.h"

CUI_GaugeBarCom::CUI_GaugeBarCom(ID3D11Device* p, ID3D11DeviceContext* c) : CComponent{ p, c } {}
CUI_GaugeBarCom::CUI_GaugeBarCom(const CUI_GaugeBarCom& Pr) : CComponent(Pr), m_fMax{ Pr.m_fMax }, m_fCur{ Pr.m_fCur }
{
}

HRESULT CUI_GaugeBarCom::Initialize_Prototype() { return S_OK; }

HRESULT CUI_GaugeBarCom::Initialize(void* pArg)
{
    if (pArg)
    {
        GAUGEBAR_DESC* pDesc = static_cast<GAUGEBAR_DESC*>(pArg);
        m_fMax = max(1.f, pDesc->fMaxValue);
        m_fCur = clamp(pDesc->fCurrent, 0.f, m_fMax);
    }
    return S_OK;
}

void CUI_GaugeBarCom::Appear(_float fCurrent, _float fMax)
{
    m_fMax = max(1.f, fMax);
    m_fCur = clamp(fCurrent, 0.f, m_fMax);
    if (m_pGauge) m_pGauge->Play_AppearSweep(m_fCur / m_fMax);
}

_bool CUI_GaugeBarCom::Is_Appearing() const
{
    return m_pGauge ? m_pGauge->Is_AppearSweeping() : false;
}

void CUI_GaugeBarCom::Reset_Empty() 
{ 
    m_fCur = 0.f; 
    if (m_pGauge) 
        m_pGauge->Reset_Empty(); 
}

void CUI_GaugeBarCom::Snap_Value(_float fCurrent, _float fMax)
{
    m_fMax = max(1.f, fMax);
    m_fCur = clamp(fCurrent, 0.f, m_fMax);
    if (m_pGauge)
        m_pGauge->Snap(m_fCur / m_fMax);
}

void CUI_GaugeBarCom::Push()
{
    if (m_pGauge)
        m_pGauge->Set_TargetRatio(m_fCur / m_fMax);
}

CUI_GaugeBarCom* CUI_GaugeBarCom::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_GaugeBarCom* pInstance = new CUI_GaugeBarCom(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_KirbyStatus");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CUI_GaugeBarCom* CUI_GaugeBarCom::Clone(void* pArg)
{
    CUI_GaugeBarCom* pInstance = new CUI_GaugeBarCom(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_GaugeBarCom");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_GaugeBarCom::Free()
{
    __super::Free();
}
