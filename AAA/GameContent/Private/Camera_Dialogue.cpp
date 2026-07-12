#include "Camera_Dialogue.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"

CCamera_Dialogue::CCamera_Dialogue(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCamera_Shakeable(pDevice, pContext) {
}
CCamera_Dialogue::CCamera_Dialogue(const CCamera_Dialogue& Prototype)
    : CCamera_Shakeable(Prototype) {
}

HRESULT CCamera_Dialogue::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    m_bActive = false;    // 평상시 휴면, 디렉터가 깨움
    return S_OK;
}

void CCamera_Dialogue::Begin(const _float4x4& AnchorWorld, _fvector vPosA, _fvector vPosB)
{
    // 배우는 대화 중 고정이므로 여기서 월드공간 샷을 전부 확정한다
    _matrix Anchor = XMLoadFloat4x4(&AnchorWorld);
    _vector vLook = XMVector3Normalize(XMVectorSetY(Anchor.r[2], 0.f));
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
    _vector vMid = XMVectorScale(XMVectorAdd(vPosA, vPosB), 0.5f);

    // 클로즈업도 전부 같은 사이드에서 잡아 시선 방향 일관성(180도 규칙) 유지
    auto MakeShot = [&](_fvector vFocus, _float fDist, _float fFovDeg)
        {
            SHOT t{};
            XMStoreFloat3(&t.vEye, vFocus + vRight * fDist + vUp * m_fEyeHeight);
            XMStoreFloat3(&t.vAim, vFocus + vUp * m_fAimHeight);
            t.fFovy = XMConvertToRadians(fFovDeg);
            return t;
        };

    m_Shots[L"Two"] = MakeShot(vMid, m_fTwoDist, 35.f);
    m_Shots[L"A"] = MakeShot(vPosA, m_fCloseDist, 30.f);
    m_Shots[L"B"] = MakeShot(vPosB, m_fCloseDist, 30.f);

    Set_Shot(L"Two", 0.f);    // 진입은 컷 전환(암전 중이라 안 보임)
    Apply_Pose();
    Update_PipeLine();
}

void CCamera_Dialogue::Set_Shot(const _wstring& strName, _float fBlendDur)
{
    auto it = m_Shots.find(strName);
    if (it == m_Shots.end())
    {
        OutputDebugStringW((L"[Camera_Dialogue] unknown shot: " + strName + L"\n").c_str());
        return;
    }

    m_tFrom = m_tCur;
    m_tTo = it->second;
    m_fBlendTotal = max(fBlendDur, 0.f);
    m_fBlendTime = 0.f;

    if (m_fBlendTotal <= 0.f)
        m_tCur = m_tFrom = m_tTo;
}

void CCamera_Dialogue::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    Tick_Shake(fTimeDelta);

    if (m_fBlendTime < m_fBlendTotal)
    {
        m_fBlendTime += fTimeDelta;
        _float t = min(1.f, m_fBlendTime / m_fBlendTotal);
        _float s = t * t * (3.f - 2.f * t);    // smoothstep 이징
        XMStoreFloat3(&m_tCur.vEye, XMVectorLerp(XMLoadFloat3(&m_tFrom.vEye), XMLoadFloat3(&m_tTo.vEye), s));
        XMStoreFloat3(&m_tCur.vAim, XMVectorLerp(XMLoadFloat3(&m_tFrom.vAim), XMLoadFloat3(&m_tTo.vAim), s));
        m_tCur.fFovy = m_tFrom.fFovy + (m_tTo.fFovy - m_tFrom.fFovy) * s;
    }

    Apply_Pose();
    __super::Priority_Update(fTimeDelta);
}

HRESULT CCamera_Dialogue::Ready_Events()
{
    if (FAILED(Ready_ShakeEvents()))
        return E_FAIL;

    Subscribe_Event(EventTag::Dialogue_CamShot, [this](void* p)
        {
            if (!m_bActive || nullptr == p)
                return;

            auto d = static_cast<DIALOGUE_CAMSHOT_DESC*>(p);
            Set_Shot(d->strShot, d->fBlendDur < 0.f ? m_fShotBlend : d->fBlendDur);
        });

    return S_OK;
}

void CCamera_Dialogue::Apply_Pose()
{
    _vector vEye = XMLoadFloat3(&m_tCur.vEye);
    _vector vLook = XMVector3Normalize(XMLoadFloat3(&m_tCur.vAim) - vEye);
    _vector vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
    _vector vUp = XMVector3Cross(vLook, vRight);

    _matrix CamWorld;
    CamWorld.r[0] = XMVectorSetW(vRight, 0.f);
    CamWorld.r[1] = XMVectorSetW(vUp, 0.f);
    CamWorld.r[2] = XMVectorSetW(vLook, 0.f);
    CamWorld.r[3] = XMVectorSetW(vEye, 1.f);
    Apply_Shake(CamWorld);
    m_pTransformCom->Set_WorldMatrix(CamWorld);

    m_fFovy = m_tCur.fFovy;
    Recalculate_ProjMatrix();
}

CCamera_Dialogue* CCamera_Dialogue::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCamera_Dialogue* p = new CCamera_Dialogue(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CCamera_Dialogue"); Safe_Release(p); }
    return p;
}
CGameObject* CCamera_Dialogue::Clone(void* pArg)
{
    CCamera_Dialogue* p = new CCamera_Dialogue(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CCamera_Dialogue"); Safe_Release(p); }
    return p;
}
void CCamera_Dialogue::Free() { __super::Free(); }