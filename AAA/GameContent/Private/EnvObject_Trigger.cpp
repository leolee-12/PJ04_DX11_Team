#include "EnvObject_Trigger.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CEnvObject_Trigger::CEnvObject_Trigger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
	, m_vAreaCenter{ 0.f, 0.f, 0.f }
	, m_vAreaSize{ 1.f, 1.f, 1.f }
	, m_vAreaRot{ 0.f, 0.f, 0.f, 1.f }
	, m_bDebugDrawTrigger{ true }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvObject_Trigger::CEnvObject_Trigger(const CEnvObject_Trigger& Prototype)
	: CEnvObject(Prototype)
	, m_strTriggerId{ Prototype.m_strTriggerId }
	, m_vAreaCenter{ Prototype.m_vAreaCenter }
	, m_vAreaSize{ Prototype.m_vAreaSize }
	, m_vAreaRot{ Prototype.m_vAreaRot }
	, m_bDebugDrawTrigger{ Prototype.m_bDebugDrawTrigger }
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

	const auto AbsAxis = [](_float fValue) -> _float
		{
			return fValue < 0.f ? -fValue : fValue;
		};

	const _float3 vAreaSize =
	{
			AbsAxis(m_vAreaSize.x),
			AbsAxis(m_vAreaSize.y),
			AbsAxis(m_vAreaSize.z)
	};

	constexpr _float kMinTriggerAxis = 0.001f;
	const _bool bValidArea =
		vAreaSize.x > kMinTriggerAxis &&
		vAreaSize.y > kMinTriggerAxis &&
		vAreaSize.z > kMinTriggerAxis;

	m_pCollider->Set_Enabled(bValidArea);

	if (bValidArea)
	{
		// 트리거는 현재 AABB만 지원 : AreaRot 무시(추후 OBB 지원할 때 받기)
		const _matrix TriggerLocalMatrix =
			XMMatrixScaling(vAreaSize.x, vAreaSize.y, vAreaSize.z) *
			XMMatrixTranslation(m_vAreaCenter.x, m_vAreaCenter.y, m_vAreaCenter.z);

		m_pCollider->Update(TriggerLocalMatrix * XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		if (m_bDebugDrawTrigger)
			m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
#endif
	}
}


void CEnvObject_Trigger::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
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

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CEnvObject_Trigger::SetUp_Collider_Callback()
{
	m_pCollider->Set_OnEnter([this](CCollider* pOther) { OnTriggerEnter(pOther); });

	m_pCollider->Set_OnStay([this](CCollider* pOther) { OnTriggerStay(pOther); });

	m_pCollider->Set_OnExit([this](CCollider* pOther) { OnTriggerExit(pOther); });
}

void CEnvObject_Trigger::Free()
{
	__super::Free();
}

NS_END
