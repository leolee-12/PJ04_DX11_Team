#include "UI_MovableContainer.h"

CUIMovableContainer::CUIMovableContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject{ pDevice, pContext }
{
}

CUIMovableContainer::CUIMovableContainer(const CUIMovableContainer& Prototype)
    : CUIContainerObject(Prototype)
    , m_vMoveTargetPos(Prototype.m_vMoveTargetPos)
    , m_fMoveTargetTiltX(Prototype.m_fMoveTargetTiltX)
    , m_fMoveTargetTiltY(Prototype.m_fMoveTargetTiltY)
    , m_fMoveDuration(Prototype.m_fMoveDuration)
{
}

HRESULT CUIMovableContainer::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUIMovableContainer::Initialize(void* pArg)
{
    CGameObject::GAMEOBJECT_DESC Default{};
    void* pDesc = pArg ? pArg : &Default;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    return S_OK;
}

void CUIMovableContainer::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    Update_IntroAnim(fTimeDelta);
    Update_MoveAnim(fTimeDelta);   // 위치/틸트 이동 진행 후 엔진 파트 갱신으로

    __super::Update(fTimeDelta);
}

void CUIMovableContainer::Start_Move(_bool bReverse)
{
    if (!m_bHomeCaptured)
    {
        _vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
        XMStoreFloat3(&m_vHomePos, vCurPos);
        m_fHomeTiltX = m_fTiltXDeg;
        m_fHomeTiltY = m_fTiltYDeg;
        m_bHomeCaptured = true;
    }

    if (!bReverse)
        Move_To(m_vMoveTargetPos, m_fMoveTargetTiltX, m_fMoveTargetTiltY, m_fMoveDuration);
    else
        Move_To(m_vHomePos, m_fHomeTiltX, m_fHomeTiltY, m_fMoveDuration);
}

void CUIMovableContainer::Move_To(const _float3& vPos, _float fTiltXDeg, _float fTiltYDeg, _float fDuration)
{
    _vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
    XMStoreFloat3(&m_vMoveFromPos, vCurPos);
    m_fMoveFromTiltX = m_fTiltXDeg;
    m_fMoveFromTiltY = m_fTiltYDeg;

    m_vMoveToPos = vPos;
    m_fMoveToTiltX = fTiltXDeg;
    m_fMoveToTiltY = fTiltYDeg;

    m_fMoveDur = (fDuration > 0.f) ? fDuration : 0.0001f;
    m_fMoveElapsed = 0.f;
    m_bMoving = true;
}

void CUIMovableContainer::Play_Intro(_float fStartScaleMul, _float fAppearDuration, _float fWaitDuration)
{
    _float3 vScale = m_pTransformCom->Get_Scaled();
    m_vIntroBaseScale = vScale;                                   // 정상(저작) 스케일 기억
    m_fIntroStartScaleMul = (fStartScaleMul > 0.f) ? fStartScaleMul : 1.f;
    m_fIntroAppearDur = (fAppearDuration > 0.f) ? fAppearDuration : 0.0001f;
    m_fIntroWaitDur = (fWaitDuration > 0.f) ? fWaitDuration : 0.f;
    m_fIntroElapsed = 0.f;
    m_eIntro = EINTRO::APPEAR;

    // 즉시 크게 세팅(깜빡임 방지) + 페이드인 시작
    m_pTransformCom->Set_Scale(vScale.x * m_fIntroStartScaleMul,
        vScale.y * m_fIntroStartScaleMul,
        vScale.z);
    Play_IntroFade(m_fIntroAppearDur);
}

void CUIMovableContainer::Update_MoveAnim(_float fTimeDelta)
{
    if (!m_bMoving)
        return;

    m_fMoveElapsed += fTimeDelta;

    _float t = m_fMoveElapsed / m_fMoveDur;
    if (t >= 1.f) t = 1.f;

    // smoothstep 이징 (ease-in-out) 으로 부드럽게
    _float e = t * t * (3.f - 2.f * t);

    // 위치 보간
    _vector vFrom = XMLoadFloat3(&m_vMoveFromPos);
    _vector vTo = XMLoadFloat3(&m_vMoveToPos);
    _vector vPos = XMVectorLerp(vFrom, vTo, e);
    vPos = XMVectorSetW(vPos, 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);

    // 틸트 보간 (도 단위 선형). 값만 갱신하면 Late_Update 의 Update_TiltedWorldMatrix 가 반영
    m_fTiltXDeg = m_fMoveFromTiltX + (m_fMoveToTiltX - m_fMoveFromTiltX) * e;
    m_fTiltYDeg = m_fMoveFromTiltY + (m_fMoveToTiltY - m_fMoveFromTiltY) * e;

    if (t >= 1.f)
        m_bMoving = false;   // 도착. 값은 목표에 스냅됨
}

void CUIMovableContainer::Update_IntroAnim(_float fTimeDelta)
{
    if (m_eIntro == EINTRO::NONE || m_eIntro == EINTRO::DONE)
        return;

    if (m_eIntro == EINTRO::APPEAR)
    {
        m_fIntroElapsed += fTimeDelta;

        _float t = m_fIntroElapsed / m_fIntroAppearDur;
        if (t >= 1.f) t = 1.f;

        // easeOutBack: 빠르게 도착하며 살짝 오버슈트(스쿼시) 후 정착 = 쾅 느낌
        const _float c1 = 1.70158f;
        const _float c3 = c1 + 1.f;
        _float p = t - 1.f;
        _float e = 1.f + c3 * p * p * p + c1 * p * p;

        _float s = m_fIntroStartScaleMul + (1.f - m_fIntroStartScaleMul) * e;
        m_pTransformCom->Set_Scale(m_vIntroBaseScale.x * s,
            m_vIntroBaseScale.y * s,
            m_vIntroBaseScale.z);

        if (t >= 1.f)
        {
            m_pTransformCom->Set_Scale(m_vIntroBaseScale.x, m_vIntroBaseScale.y, m_vIntroBaseScale.z);
            m_fIntroElapsed = 0.f;
            m_eIntro = EINTRO::WAIT;
        }
        return;
    }

    if (m_eIntro == EINTRO::WAIT)
    {
        m_fIntroElapsed += fTimeDelta;
        if (m_fIntroElapsed >= m_fIntroWaitDur)
        {
            m_eIntro = EINTRO::DONE;
            On_IntroFinished();   // 파생: 이벤트 Publish / 기본: std::function 호출
            Start_Move(false);    // 도착 지점으로 이동
        }
        return;
    }
}

json CUIMovableContainer::Serialize() const
{
    json j = __super::Serialize();
    j["MoveTargetPos"] = { m_vMoveTargetPos.x, m_vMoveTargetPos.y, m_vMoveTargetPos.z };
    j["MoveTargetTiltX"] = m_fMoveTargetTiltX;
    j["MoveTargetTiltY"] = m_fMoveTargetTiltY;
    j["MoveDuration"] = m_fMoveDuration;
    return j;
}

void CUIMovableContainer::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);

    if (j.contains("MoveTargetPos") && j["MoveTargetPos"].is_array() && j["MoveTargetPos"].size() == 3)
    {
        m_vMoveTargetPos.x = j["MoveTargetPos"][0].get<_float>();
        m_vMoveTargetPos.y = j["MoveTargetPos"][1].get<_float>();
        m_vMoveTargetPos.z = j["MoveTargetPos"][2].get<_float>();
    }
    if (j.contains("MoveTargetTiltX")) m_fMoveTargetTiltX = j["MoveTargetTiltX"].get<_float>();
    if (j.contains("MoveTargetTiltY")) m_fMoveTargetTiltY = j["MoveTargetTiltY"].get<_float>();
    if (j.contains("MoveDuration"))    m_fMoveDuration = j["MoveDuration"].get<_float>();
}

CUIMovableContainer* CUIMovableContainer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUIMovableContainer* pInstance = new CUIMovableContainer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUIMovableContainer");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUIMovableContainer::Clone(void* pArg)
{
    CUIMovableContainer* pInstance = new CUIMovableContainer(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUIMovableContainer");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUIMovableContainer::Free()
{
    __super::Free();
}