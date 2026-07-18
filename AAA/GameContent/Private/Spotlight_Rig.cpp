#include "Spotlight_Rig.h"
#include "GameInstance_Proxy.h"

NS_BEGIN(Client)

HRESULT CSpotlight_Rig::Initialize(CGameInstance_Proxy* pProxy, const LIGHT_DESC& tInit)
{
    if (nullptr == pProxy)
        return E_FAIL;

    m_pProxy = pProxy;
    m_tDesc = tInit;
    m_tDesc.eType = LIGHT::SPOT;

    m_iIndex = m_pProxy->Add_Light(m_tDesc);
    if (m_iIndex < 0)
        return E_FAIL;

    return S_OK;
}

void CSpotlight_Rig::Set_Position(_fvector vPos)
{
    _float3 p{}; XMStoreFloat3(&p, vPos);
    m_tDesc.vPosition = _float4(p.x, p.y, p.z, 1.f);
}

void CSpotlight_Rig::Aim_At(_fvector vTargetPos)
{
    _vector vPos = XMLoadFloat4(&m_tDesc.vPosition);
    Set_Direction(vTargetPos - vPos);
}

void CSpotlight_Rig::Set_Direction(_fvector vDir)
{
    _float3 d{}; XMStoreFloat3(&d, XMVector3Normalize(vDir));
    m_tDesc.vDirection = _float4(d.x, d.y, d.z, 0.f);
}

void CSpotlight_Rig::Set_Color(const _float3& vColor, _float fIntensity)
{
    m_vColor = vColor;          
    m_fIntensity = fIntensity;
}

void CSpotlight_Rig::Set_Cone(_float fInnerDeg, _float fOuterDeg)
{
    m_fInnerDeg = fInnerDeg;
    m_fOuterDeg = fOuterDeg;
}

void CSpotlight_Rig::Set_Range(_float fRange) { m_tDesc.fRange = fRange; }

void CSpotlight_Rig::Set_Enabled(_bool bOn)
{
    m_iFadeDir = bOn ? +1 : -1;
}

void CSpotlight_Rig::Push()
{
    if (m_pProxy->Is_EditMode())
        return;

    if (m_iIndex < 0)
        return;

    m_pProxy->Set_LightDesc(static_cast<_uint>(m_iIndex), m_tDesc);

    const _float fEnable = (m_fCurFade > 0.001f) ? 1.f : 0.f;
    m_pProxy->Set_ShaderGlobal("g_vGodRaySpotPos",
        _float4(m_tDesc.vPosition.x, m_tDesc.vPosition.y, m_tDesc.vPosition.z, m_tDesc.fRange));
    m_pProxy->Set_ShaderGlobal("g_vGodRaySpotDir",
        _float4(m_tDesc.vDirection.x, m_tDesc.vDirection.y, m_tDesc.vDirection.z, fEnable));
    m_pProxy->Set_ShaderGlobal("g_vGodRaySpotColor",
        _float4(m_tDesc.vDiffuse.x * m_fGodRayStrength,
            m_tDesc.vDiffuse.y * m_fGodRayStrength,
            m_tDesc.vDiffuse.z * m_fGodRayStrength, 0.f));
    m_pProxy->Set_ShaderGlobal("g_vGodRaySpotCone",
        _float4(m_tDesc.fInnerCos, m_tDesc.fOuterCos, m_fBeamDensity, m_fGodRayNear));
}

void CSpotlight_Rig::Set_Target(_fvector vFocusPos)
{
    XMStoreFloat3(&m_vFocusTarget, vFocusPos);

    if (!m_bFocusInit)         
    {
        XMStoreFloat3(&m_vFocusCurrent, vFocusPos);
        m_bFocusInit = true;
    }
}

void CSpotlight_Rig::Set_OverheadOffset(_fvector vOffset)
{
    XMStoreFloat3(&m_vOverheadOffset, vOffset);
    m_bOverhead = (XMVectorGetX(XMVector3LengthSq(vOffset)) > 1e-6f);
}

void CSpotlight_Rig::Snap()
{
    m_vFocusCurrent = m_vFocusTarget;
    m_bFocusInit = true;
}

void CSpotlight_Rig::Snap_Fade(_bool bOn)
{
    m_fFadeT = bOn ? 1.f : 0.f;
    m_iFadeDir = 0;
    Apply_Fade(0.f);
}

void CSpotlight_Rig::Update(_float fTimeDelta)
{
    Apply_Fade(fTimeDelta);

    _vector vCur = XMLoadFloat3(&m_vFocusCurrent);
    _vector vTgt = XMLoadFloat3(&m_vFocusTarget);
    const _float fT = 1.f - expf(-m_fFollowSpeed * fTimeDelta);
    vCur = XMVectorLerp(vCur, vTgt, fT);
    XMStoreFloat3(&m_vFocusCurrent, vCur);

    Set_Direction(vCur - XMLoadFloat4(&m_tDesc.vPosition));
    Push();
}

void CSpotlight_Rig::Apply_Fade(_float fTimeDelta)
{
    if (m_iFadeDir != 0)
    {
        const _float step = (m_fFadeDuration > 1e-4f) ? (fTimeDelta / m_fFadeDuration) : 1.f;
        m_fFadeT += m_iFadeDir * step;
        if (m_fFadeT <= 0.f) { m_fFadeT = 0.f; m_iFadeDir = 0; }
        else if (m_fFadeT >= 1.f) { m_fFadeT = 1.f; m_iFadeDir = 0; }
    }

    const _float t = m_fFadeT * m_fFadeT * (3.f - 2.f * m_fFadeT);
    m_fCurFade = t;

    const _float fInner = m_fFadeStartInnerDeg + (m_fInnerDeg - m_fFadeStartInnerDeg) * t;
    const _float fOuter = m_fFadeStartOuterDeg + (m_fOuterDeg - m_fFadeStartOuterDeg) * t;
    m_tDesc.fInnerCos = cosf(XMConvertToRadians(fInner) * 0.5f);
    m_tDesc.fOuterCos = cosf(XMConvertToRadians(fOuter) * 0.5f);

    const _float f = m_fIntensity * t;
    m_tDesc.vDiffuse = _float4(m_vColor.x * f, m_vColor.y * f, m_vColor.z * f, 1.f);
}

CSpotlight_Rig* CSpotlight_Rig::Create(CGameInstance_Proxy* pProxy, const LIGHT_DESC& tInit)
{
    CSpotlight_Rig* pInstance = new CSpotlight_Rig();

    if (FAILED(pInstance->Initialize(pProxy, tInit)))
    {
        MSG_BOX("Failed to Created : CSpotlight_Rig");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSpotlight_Rig::Free()
{
    __super::Free();
}

NS_END