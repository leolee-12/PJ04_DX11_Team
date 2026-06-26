#include "Monster_RailMovement.h"
#include "Transform.h"
#include "LevelDesign_Rail.h"

CMonster_RailMovement::CMonster_RailMovement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster_Movement{ pDevice, pContext }
{
}

CMonster_RailMovement::CMonster_RailMovement(const CMonster_RailMovement& Prototype)
	: CMonster_Movement (Prototype)
{
}

HRESULT CMonster_RailMovement::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CMonster_RailMovement::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CMonster_RailMovement::Set_Rail(const LD_RAIL_DESC& Desc)
{
	m_tRailDesc     = Desc;
	m_bHasRail      = true;
    m_fPathLength   = Compute_PathLength();
    m_fPathS        = 0.f;
    m_iPathDir      = 1;
}

void CMonster_RailMovement::Clear_Rail()
{
	m_bHasRail = false;
}

_bool CMonster_RailMovement::Update_RailFollow(_float fTimeDelta)
{
    if (!m_bHasRail || nullptr == m_pTransform)
        return false;

    if (m_fPathLength <= 1e-4f)
        return false;

    if (fabsf(m_fSpinSpeedDeg) > 1e-4f)
    {
        m_fSpinAngle += XMConvertToRadians(m_fSpinSpeedDeg) * fTimeDelta;
        if (m_fSpinAngle >= XM_2PI)
            m_fSpinAngle -= XM_2PI;

        m_pTransform->Rotation(
            XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fSpinAngle);
    }

    m_fPathS += m_iPathDir * m_fPathSpeed * fTimeDelta;

    const _bool bLoop =
        (m_tRailDesc.fRadius > 0.001f) || m_tRailDesc.bClose;

    if (bLoop)
    {
        while (m_fPathS >= m_fPathLength)
            m_fPathS -= m_fPathLength;
        while (m_fPathS < 0.f)
            m_fPathS += m_fPathLength;
    }
    else
    {
        if (m_fPathS > m_fPathLength)
        {
            m_fPathS = m_fPathLength
                - (m_fPathS - m_fPathLength);
            m_iPathDir = -1;
        }
        else if (m_fPathS < 0.f)
        {
            m_fPathS = -m_fPathS;
            m_iPathDir = 1;
        }
    }

    _float3 vTarget{};
    if (!Eval_PathPos(m_fPathS, &vTarget))
        return false;

    m_pTransform->Set_State(STATE::POSITION, XMVectorSet(vTarget.x, vTarget.y, vTarget.z, 1.f));

    Sync_To_Controller();
    return true;
}

_bool CMonster_RailMovement::Is_OffPath(_float fThreshold) const
{
    if (!m_bHasRail || nullptr == m_pTransform)
        return false;
    if (m_fPathLength <= 1e-4f)
        return false;

    _float3 vRail{};
    if (!Eval_PathPos(m_fPathS, &vRail))
        return false;

    _float3 vCur{};
    XMStoreFloat3(&vCur,
        m_pTransform->Get_State(STATE::POSITION));

    const _float dx = vCur.x - vRail.x;
    const _float dy = vCur.y - vRail.y;
    const _float dz = vCur.z - vRail.z;

    return (dx * dx + dy * dy + dz * dz)
        > (fThreshold * fThreshold);
}

void CMonster_RailMovement::Warp_ToRandomPoint()
{
    if (!m_bHasRail || nullptr == m_pTransform)
        return;
    if (m_fPathLength <= 1e-4f)
        return;

    const _float u = static_cast<_float>(rand()) / static_cast<_float>(RAND_MAX);
    m_fPathS = u * m_fPathLength;
    m_iPathDir = 1;

    _float3 vTarget{};
    if (!Eval_PathPos(m_fPathS, &vTarget))
        return;

    m_pTransform->Set_State(STATE::POSITION,
        XMVectorSet(vTarget.x, vTarget.y, vTarget.z, 1.f));

    Sync_To_Controller();
}

void CMonster_RailMovement::Calc_Vertical(_float fTimeDelta)
{
    if (!Is_Launched())
    {
        m_fVerticalVelocity = 0.f;
        return;
    }

    __super::Calc_Vertical(fTimeDelta);
}

_float  CMonster_RailMovement::Compute_PathLength() const
{
    const _uint segs =
        CLevelDesign_Rail::Get_SegmentCount(m_tRailDesc);
    if (0 == segs)
        return 0.f;

    if (m_tRailDesc.fRadius > 0.001f)
        return XM_2PI * m_tRailDesc.fRadius;

    _float sum = 0.f;
    for (_uint s = 0; s < segs; ++s)
    {
        _float3 a{}, b{};
        CLevelDesign_Rail::Evaluate_Segment(m_tRailDesc, s, 0.f, &a);
        CLevelDesign_Rail::Evaluate_Segment(m_tRailDesc, s, 1.f, &b);
        const _float dx = b.x - a.x;
        const _float dy = b.y - a.y;
        const _float dz = b.z - a.z;
        sum += sqrtf(dx * dx + dy * dy + dz * dz);
    }
    return sum;
}

_bool CMonster_RailMovement::Eval_PathPos(_float fS, _float3* pOut) const
{
    if (nullptr == pOut)
        return false;

    const _uint segs = CLevelDesign_Rail::Get_SegmentCount(m_tRailDesc);
    if (0 == segs)
        return false;

    if (m_tRailDesc.fRadius > 0.001f)
    {
        const _float L = XM_2PI * m_tRailDesc.fRadius;
        const _float t = (L > 1e-4f) ? (fS / L) : 0.f;
        return CLevelDesign_Rail::Evaluate_Segment(
            m_tRailDesc, 0, t, pOut);
    }

    _float rem = fS;
    for (_uint i = 0; i < segs; ++i)
    {
        _float3 a{}, b{};
        CLevelDesign_Rail::Evaluate_Segment(
            m_tRailDesc, i, 0.f, &a);
        CLevelDesign_Rail::Evaluate_Segment(
            m_tRailDesc, i, 1.f, &b);
        const _float dx = b.x - a.x;
        const _float dy = b.y - a.y;
        const _float dz = b.z - a.z;
        const _float segLen =
            sqrtf(dx * dx + dy * dy + dz * dz);

        if (rem <= segLen || i + 1 == segs)
        {
            const _float t =
                (segLen > 1e-4f) ? (rem / segLen) : 0.f;
            return CLevelDesign_Rail::Evaluate_Segment(
                m_tRailDesc, i, t, pOut);
        }
        rem -= segLen;
    }
    return false;
}

CMonster_RailMovement* CMonster_RailMovement::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMonster_RailMovement* pInstance = new CMonster_RailMovement(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMonster_RailMovement");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CMonster_RailMovement::Clone(void* pArg)
{
    CMonster_RailMovement* pInstance = new CMonster_RailMovement(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CMonster_RailMovement");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMonster_RailMovement::Free()
{
    __super::Free();
}