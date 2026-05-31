#include "Transform.h"
#include "Shader.h"
#include "GameInstance.h"

CTransform::CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent(pDevice, pContext)
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
}

CTransform::CTransform(const CTransform& Prototype)
    : CComponent(Prototype)
{
}

HRESULT CTransform::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{
    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

    if (nullptr == pArg)
        return S_OK;

    auto    pDesc = static_cast<TRANSFORM_DESC*>(pArg);

    m_fRotationPerSec = pDesc->fRotationPerSec;
    m_fSpeedPerSec = pDesc->fSpeedPerSec;

	Set_State(STATE::RIGHT,     XMLoadFloat4(&(pDesc->vRight)));
	Set_State(STATE::UP,        XMLoadFloat4(&(pDesc->vUp)));
	Set_State(STATE::LOOK,      XMLoadFloat4(&(pDesc->vLook)));
	Set_State(STATE::POSITION,  XMLoadFloat4(&(pDesc->vPosition)));

    return S_OK;
}

HRESULT CTransform::Bind_ShaderResource(CShader* pShader, const _char* pConstantName)
{
    return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);
}

void CTransform::Set_Scale(_float fScaleX, _float fScaleY, _float fScaleZ)
{
    Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * fScaleX);
    Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * fScaleY);
    Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * fScaleZ);
}

void CTransform::Scaling(_float fScaleX, _float fScaleY, _float fScaleZ)
{
    Set_State(STATE::RIGHT, Get_State(STATE::RIGHT) * fScaleX);
    Set_State(STATE::UP, Get_State(STATE::UP) * fScaleY);
    Set_State(STATE::LOOK, Get_State(STATE::LOOK) * fScaleZ);
}

void CTransform::Go_Straight(_float fTimeDelta, CNavigation* pNavigation)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vLook = Get_State(STATE::LOOK);

    vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

    if (nullptr == pNavigation ||
        true == pNavigation->Query_isMove(vPosition))
        Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Backward(_float fTimeDelta, CNavigation* pNavigation)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vLook = Get_State(STATE::LOOK);

    vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

    if (nullptr == pNavigation ||
        true == pNavigation->Query_isMove(vPosition))
        Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Left(_float fTimeDelta)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vRight = Get_State(STATE::RIGHT);

    vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Right(_float fTimeDelta)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vRight = Get_State(STATE::RIGHT);

    vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

_bool CTransform::Go_ToPoint(_fvector vTargetPos, _float fTimeDelta, CNavigation* pNavigation)
{
    _vector vPos = Get_State(STATE::POSITION);
    _vector vToTarget = XMVectorSetY(XMVectorSubtract(vTargetPos, vPos), 0.f);
    _float  fDist = XMVectorGetX(XMVector3Length(vToTarget));
    _float  fStep = m_fSpeedPerSec * fTimeDelta;

    if (fDist <= fStep)
    {
        _vector vSnap = XMVectorSetW(vTargetPos, 1.f);
        if (nullptr == pNavigation || pNavigation->Query_isMove(vSnap))
        {
            Set_State(STATE::POSITION, vSnap);
        }
        else
        {
            _vector vMove = XMVectorSubtract(vSnap, vPos);
            _vector vSlideMove;
            if (pNavigation->Compute_SlideVector(vPos, vMove, &vSlideMove))
            {
                _vector vSlideTarget = XMVectorSetW(XMVectorAdd(vPos, vSlideMove), 1.f);
                if (pNavigation->Query_isMove(vSlideTarget))
                    Set_State(STATE::POSITION, vSlideTarget);
            }
        }
        return true;
    }

    LookAt_Smooth(vTargetPos, fTimeDelta);

    _vector vDir = XMVector3Normalize(vToTarget);
    _vector vNewPos = XMVectorSetW(XMVectorAdd(vPos, XMVectorScale(vDir, fStep)), 1.f);

    if (nullptr == pNavigation || pNavigation->Query_isMove(vNewPos))
    {
        Set_State(STATE::POSITION, vNewPos);
    }
    else
    {
        _vector vMove = XMVectorSubtract(vNewPos, vPos);
        _vector vSlideMove;
        if (pNavigation->Compute_SlideVector(vPos, vMove, &vSlideMove))
        {
            _vector vSlideTarget = XMVectorSetW(XMVectorAdd(vPos, vSlideMove), 1.f);
            if (pNavigation->Query_isMove(vSlideTarget))
                Set_State(STATE::POSITION, vSlideTarget);
        }
    }

    return false;
}

_bool CTransform::Follow_Waypoints(deque<_float3>& Waypoints, _float fTimeDelta, CNavigation* pNavi)
{
    _float fRemaining = fTimeDelta;

    while (!Waypoints.empty() && fRemaining > FLT_EPSILON)
    {
        _vector vTarget = XMLoadFloat3(&Waypoints.front());
        _vector vPos = Get_State(STATE::POSITION);
        _vector vToTarget = XMVectorSetY(XMVectorSubtract(vTarget, vPos), 0.f);
        _float  fDist = XMVectorGetX(XMVector3Length(vToTarget));
        _float  fStep = m_fSpeedPerSec * fRemaining;

        if (fDist > fStep)
        {
            Go_ToPoint(vTarget, fRemaining, pNavi);
            return false;
        }

        _float fUsed = fDist / m_fSpeedPerSec;
        Go_ToPoint(vTarget, fUsed, pNavi);
        Waypoints.pop_front();
        fRemaining -= fUsed;
    }

    return Waypoints.empty();
}

void CTransform::Go_Dir(_float fTimeDelta, _fvector vDir)
{
    _vector     vPosition = Get_State(STATE::POSITION);

    vPosition += XMVectorSetW(XMVector3Normalize(vDir), 0.f) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Rotation(_fvector vAxis, _float fRadian)
{
    _float3         vScaled = Get_Scaled();

    _vector         vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScaled.x;
    _vector         vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScaled.y;
    _vector         vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScaled.z;

    _matrix         RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

    Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix));
    Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix));
}

void CTransform::Rotation(_fvector vQuatanion)
{
    _float3         vScaled = Get_Scaled();

    _vector         vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScaled.x;
    _vector         vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScaled.y;
    _vector         vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScaled.z;

    _matrix RotationMatrix = XMMatrixRotationQuaternion(vQuatanion);
    Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix));
    Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix));
}

void CTransform::Rotate(_fvector vQuatanion)
{
    _matrix RotationMatrix = XMMatrixRotationQuaternion(vQuatanion);
    Set_State(STATE::RIGHT, XMVector3TransformNormal(Get_State(STATE::RIGHT), RotationMatrix));
    Set_State(STATE::UP, XMVector3TransformNormal(Get_State(STATE::UP), RotationMatrix));
    Set_State(STATE::LOOK, XMVector3TransformNormal(Get_State(STATE::LOOK), RotationMatrix));
}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta)
{
    Rotate(XMQuaternionRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta));
}

void CTransform::Chase(_fvector vGoal, _float fTimeDelta, _float fLimit, CNavigation* pNavigation)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vDir = vGoal - vPosition;

    _float      fDistance = XMVectorGetX(XMVector3Length(vDir));

    if (fDistance >= fLimit)
        vPosition += XMVector3Normalize(vDir) * m_fSpeedPerSec * fTimeDelta;

    if (nullptr == pNavigation ||
        true == pNavigation->Query_isMove(vPosition))
        Set_State(STATE::POSITION, vPosition);
}

void CTransform::Chase_XZ(_fvector vGoal, _float fTimeDelta, _float fLimit, CNavigation* pNavigation)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vDir = vGoal - vPosition;
    vDir = XMVectorSetY(vDir, 0.f);

    _float      fDistance = XMVectorGetX(XMVector3Length(vDir));

    if (fDistance >= fLimit)
        vPosition += XMVector3Normalize(vDir) * m_fSpeedPerSec * fTimeDelta;

    if (nullptr == pNavigation ||
        true == pNavigation->Query_isMove(vPosition))
        Set_State(STATE::POSITION, vPosition);
}

void CTransform::LookAt(_fvector vAt)
{
    _vector         vLook = vAt - Get_State(STATE::POSITION);
    _vector         vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
    _vector         vUp = XMVector3Cross(vLook, vRight);

    _float3         vScaled = Get_Scaled();

    Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScaled.x);
    Set_State(STATE::UP, XMVector3Normalize(vUp) * vScaled.y);
    Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScaled.z);
}

_bool CTransform::LookAt_Smooth(_fvector vAt, _float fTimeDelta)
{
    _vector vPos = Get_State(STATE::POSITION);
    _vector vToTarget = XMVectorSetY(XMVectorSubtract(vAt, vPos), 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vToTarget)) < FLT_EPSILON)
        return true;

    _vector vDesired = XMVector3Normalize(vToTarget);
    _vector vLook = XMVector3Normalize(XMVectorSetY(Get_State(STATE::LOOK), 0.f));

    _float fDot = XMVectorGetX(XMVector3Dot(vLook, vDesired));
    _float fCrossY = XMVectorGetZ(vLook) * XMVectorGetX(vDesired) - XMVectorGetX(vLook) * XMVectorGetZ(vDesired);
    _float fYaw = atan2f(fCrossY, fDot);

    _float fSpeed = m_fRotationPerSec * (1.f + fabsf(fYaw));

    _float fMaxStep = fSpeed * fTimeDelta;
    _float fApplied = (fabsf(fYaw) > fMaxStep)
        ? (fYaw > 0.f ? fMaxStep : -fMaxStep)
        : fYaw;

    Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApplied));

    return fabsf(fYaw) <= fMaxStep;
}

CTransform* CTransform::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTransform* pInstance = new CTransform(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTransform");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CTransform::Clone(void* pArg)
{
    CTransform* pInstance = new CTransform(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CTransform");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTransform::Free()
{
    __super::Free();

}
