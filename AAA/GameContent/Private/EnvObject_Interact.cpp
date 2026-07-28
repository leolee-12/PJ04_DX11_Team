#include "EnvObject_Interact.h"

#include "GameInstance_Proxy.h"
#include "Geometry_Utils.h"

NS_BEGIN(Client)

namespace
{
	constexpr _float GROUND_SNAP_LIFT_PADDING = 0.1f;
	constexpr _float GROUND_SNAP_MAX_DROP = 1.f;
	constexpr _float GROUND_SNAP_CLEARANCE = 0.02f;
	constexpr _float GROUND_SNAP_MIN_NORMAL_Y = 0.5f;
}

CEnvObject_Interact::CEnvObject_Interact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvObject_Interact::CEnvObject_Interact(const CEnvObject_Interact& Prototype)
	: CEnvObject(Prototype)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvObject_Interact::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_iMaterialID = 0;
	return S_OK;
}

HRESULT CEnvObject_Interact::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents(m_tDesc.iModelProtoLevel, m_tDesc.wstrModelProtoTag)))
		return E_FAIL;

	Snap_ToGround();

	if (FAILED(Rebuild_PhysicsActor()))
		return E_FAIL;

	if (FAILED(Ready_InteractComponents()))
		return E_FAIL;

	return S_OK;
}

void CEnvObject_Interact::Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
}

void CEnvObject_Interact::Late_Update(_float fTimeDelta)
{
	Refresh_WorldBounds();
	__super::Late_Update(fTimeDelta);
	Check_Visible();

	if (m_bVisible)
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);

	if (m_bVisibleShadow)
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CEnvObject_Interact::Ready_InteractComponents()
{
	return S_OK;
}

void CEnvObject_Interact::Snap_ToGround()
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    if (ENV_INTERACT_TYPE::PHYSICS_PROP != m_tDesc.eInteractType
        && ENV_INTERACT_TYPE::BREAKABLE != m_tDesc.eInteractType)
    {
        return;
    }

    const BoundingBox& LocalBounds = Get_LocalBounds();
    if (!GeometryUtils::Is_ValidAABB(LocalBounds))
        return;

    const _matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

    _vector vScale{};
    _vector vRotation{};
    _vector vPosition{};
    if (!XMMatrixDecompose(&vScale, &vRotation, &vPosition, WorldMatrix))
        return;

    _float3 vAbsScale{};
    XMStoreFloat3(&vAbsScale, XMVectorAbs(vScale));

    const _float3 vHalfExtents = {
            LocalBounds.Extents.x * vAbsScale.x,
            LocalBounds.Extents.y * vAbsScale.y,
            LocalBounds.Extents.z * vAbsScale.z };

    if (!GeometryUtils::Has_UsableSize(vHalfExtents))
        return;

    _float3 vSweepCenter{};
    XMStoreFloat3(&vSweepCenter,
        XMVector3TransformCoord(XMLoadFloat3(&LocalBounds.Center), WorldMatrix));

    const _float fLiftDistance =
        max(max(vHalfExtents.x, vHalfExtents.y), vHalfExtents.z) * 2.f
        + GROUND_SNAP_LIFT_PADDING;

    vSweepCenter.y += fLiftDistance;

    _float4 qRotation{};
    XMStoreFloat4(&qRotation, XMQuaternionNormalize(vRotation));

    const _float3 vDown = { 0.f, -1.f, 0.f };
    _float3 vHitNormal{};
    _float fHitDistance{};

    if (!m_pGameInstance_Proxy->Sweep_Box(
        vSweepCenter,
        vHalfExtents,
        qRotation,
        vDown,
        fLiftDistance + GROUND_SNAP_MAX_DROP,
        &vHitNormal,
        &fHitDistance,
        true,
        false))
    {
        return;
    }

    if (fHitDistance < 0.f || vHitNormal.y < GROUND_SNAP_MIN_NORMAL_Y)
        return;

    const _float fOffsetY = fLiftDistance - fHitDistance + GROUND_SNAP_CLEARANCE;
    vPosition = XMVectorSetY(vPosition, XMVectorGetY(vPosition) + fOffsetY);

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vPosition, 1.f));
    Refresh_WorldBounds();
}

CEnvObject_Interact* CEnvObject_Interact::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvObject_Interact* pInstance = new CEnvObject_Interact(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvObject_Interact");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvObject_Interact::Clone(void* pArg)
{
	CEnvObject_Interact* pInstance = new CEnvObject_Interact(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvObject_Interact");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Interact::Free()
{
	__super::Free();
}

NS_END