#include "EnvObject_Trigger.h"

#include "GameInstance.h"
#include "Geometry_Utils.h"

NS_BEGIN(Client)

CEnvObject_Trigger::CEnvObject_Trigger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
	, m_vAreaCenter{ 0.f, 0.f, 0.f }
	, m_vAreaSize{ 1.f, 1.f, 1.f }
	, m_vAreaRot{ 0.f, 0.f, 0.f, 1.f }
	, m_iCollisionLayer{ static_cast<_int>(COLLISION_LAYER::ENV_TRIGGER) }
	, m_bDebugDrawTrigger{ true }
	, m_strDebugTextFontTag{ L"KOR-FOT-ComicReggaeStd-B" }
	, m_vDebugTextColor{ 1.f, 1.f, 0.2f, 1.f }
	, m_fDebugTextScale{ 0.7f }
	, m_vDebugBoxColor{ 0.2f, 0.2f, 0.4f, 1.f }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvObject_Trigger::CEnvObject_Trigger(const CEnvObject_Trigger& Prototype)
	: CEnvObject(Prototype)
	, m_strTriggerId{ Prototype.m_strTriggerId }
	, m_vAreaCenter{ Prototype.m_vAreaCenter }
	, m_vAreaSize{ Prototype.m_vAreaSize }
	, m_vAreaRot{ Prototype.m_vAreaRot }
	, m_iCollisionLayer{ Prototype.m_iCollisionLayer }
	, m_bDebugDrawTrigger{ Prototype.m_bDebugDrawTrigger }
	, m_strDebugTextFontTag{ Prototype.m_strDebugTextFontTag }
	, m_vDebugTextColor{ Prototype.m_vDebugTextColor }
	, m_fDebugTextScale{ Prototype.m_fDebugTextScale }
	, m_vDebugBoxColor{ Prototype.m_vDebugBoxColor }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvObject_Trigger::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnvObject_Trigger::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strTriggerId = false == m_tDesc.wstrObjectName.empty() ? m_tDesc.wstrObjectName : m_tDesc.wstrComponentName;
	m_vAreaCenter = m_tDesc.tEffect.vAreaCenter;
	m_vAreaSize = m_tDesc.tEffect.vAreaSize;
	m_vAreaRot = m_tDesc.tEffect.vAreaRot;

	m_bTriggerShapeDirty = true;
	m_bTriggerTransformDirty = true;
	m_bTriggerDebugStyleDirty = true;
	m_bTriggerAreaValid = false;

	m_bRenderable = false;
	m_bCastShadow = false;
	m_bUseCullDistance = false;
	m_bUseCullFrustum = false;

	if (!m_tDesc.wstrModelProtoTag.empty())
	{
		if (FAILED(Ready_RenderComponents(m_tDesc.iModelProtoLevel, m_tDesc.wstrModelProtoTag)))
			return E_FAIL;
	}

	if (FAILED(Ready_TriggerCollider()))
		return E_FAIL;

	SetUp_Collider_Callback();

	return S_OK;
}

void CEnvObject_Trigger::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	Refresh_TriggerCollisionLayer();

	if (m_bTriggerShapeDirty || m_bTriggerTransformDirty)
		Refresh_TriggerCollider();

#ifdef _DEBUG
	if (m_bTriggerAreaValid && m_bDebugDrawTrigger)
	{
		if (m_bTriggerDebugStyleDirty)
			Refresh_TriggerDebugStyle();

		m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
		m_pGameInstance_Proxy->Add_DebugTextComponent(m_pCollider);
	}
#endif
}

void CEnvObject_Trigger::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

#pragma region Editable
HRESULT CEnvObject_Trigger::On_EditTransformChanged()
{
	m_bTriggerTransformDirty = true;
	return __super::On_EditTransformChanged();
}
#pragma endregion

void CEnvObject_Trigger::Mark_TriggerDirty()
{
	Refresh_TriggerCollisionLayer();
	m_bTriggerShapeDirty = true;
	m_bTriggerDebugStyleDirty = true;
}

void CEnvObject_Trigger::OnTriggerEnter(CCollider* pOther)
{
	UNREFERENCED_PARAMETER(pOther);
}

void CEnvObject_Trigger::OnTriggerStay(CCollider* pOther)
{
	UNREFERENCED_PARAMETER(pOther);
}

void CEnvObject_Trigger::OnTriggerExit(CCollider* pOther)
{
	UNREFERENCED_PARAMETER(pOther);
}

_bool CEnvObject_Trigger::Is_PlayerActivator(const CCollider* pOther) const
{
	return nullptr != pOther && ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup();
}

HRESULT CEnvObject_Trigger::Ready_TriggerCollider()
{
	m_pCollider = Add_Component<CCollider>(
		L"Com_TriggerCollider",
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));

	if (nullptr == m_pCollider)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.f, 0.f };
	ColliderDesc.vSize = { 1.f, 1.f, 1.f };

	if (FAILED(m_pCollider->Initialize(&ColliderDesc)))
		return E_FAIL;

	Refresh_TriggerCollisionLayer();
	return S_OK;
}

void CEnvObject_Trigger::Refresh_TriggerCollisionLayer()
{
	if (nullptr == m_pCollider)
		return;

	const _uint iEnvTriggerLayer = ETOUI(COLLISION_LAYER::ENV_TRIGGER);
	const _uint iDeformReleaseLayer = ETOUI(COLLISION_LAYER::DEFORM_RELEASE_AREA);
	const _uint iCollisionLayer = static_cast<_int>(iDeformReleaseLayer) == m_iCollisionLayer ? iDeformReleaseLayer : iEnvTriggerLayer;

	if (m_iCollisionLayer != static_cast<_int>(iCollisionLayer))
		m_iCollisionLayer = static_cast<_int>(iCollisionLayer);

	if (m_iRegisteredCollisionLayer == iCollisionLayer)
		return;

	if (UINT_MAX != m_iRegisteredCollisionLayer)
		m_pGameInstance_Proxy->Immediate_Unregister(m_pCollider, m_iRegisteredCollisionLayer);

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, iCollisionLayer);
	m_iRegisteredCollisionLayer = iCollisionLayer;
}

void CEnvObject_Trigger::SetUp_Collider_Callback()
{
	m_pCollider->Set_OnEnter([this](CCollider* pOther) { OnTriggerEnter(pOther); });

	m_pCollider->Set_OnStay([this](CCollider* pOther) { OnTriggerStay(pOther); });

	m_pCollider->Set_OnExit([this](CCollider* pOther) { OnTriggerExit(pOther); });
}

void CEnvObject_Trigger::Refresh_TriggerCollider()
{
	m_bTriggerAreaValid = GeometryUtils::Has_UsableSize(m_vAreaSize);
	m_pCollider->Set_Enabled(m_bTriggerAreaValid);

	if (m_bTriggerAreaValid)
	{
		const _float3 vAreaSize = GeometryUtils::Make_AbsSize(m_vAreaSize);
		const _matrix TriggerLocalMatrix =
			XMMatrixScaling(vAreaSize.x, vAreaSize.y, vAreaSize.z) *
			XMMatrixTranslation(m_vAreaCenter.x, m_vAreaCenter.y, m_vAreaCenter.z);

		m_pCollider->Update(TriggerLocalMatrix * XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	}

	m_bTriggerShapeDirty = false;
	m_bTriggerTransformDirty = false;
}

#ifdef _DEBUG
void CEnvObject_Trigger::Refresh_TriggerDebugStyle()
{
	const _float fDebugTextScale = max(0.1f, m_fDebugTextScale);
	m_pCollider->Set_DebugText(m_strDebugTextFontTag, Get_DebugLabel(), m_vDebugTextColor, _float2(fDebugTextScale,
		fDebugTextScale));
	m_pCollider->Set_DebugRenderColor(m_vDebugBoxColor);
	m_bTriggerDebugStyleDirty = false;
}

_wstring CEnvObject_Trigger::Get_DebugLabel() const
{
	if (m_strTriggerId.empty())
		return L"NULL";

	return m_strTriggerId;
}
#endif

void CEnvObject_Trigger::Free()
{
	if (nullptr != m_pCollider)
	{
		m_pCollider->Set_OnEnter(nullptr);
		m_pCollider->Set_OnStay(nullptr);
		m_pCollider->Set_OnExit(nullptr);
	}

	__super::Free();
}

NS_END
