#include "EnvObject_Trigger.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CEnvObject_Trigger::CEnvObject_Trigger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvObject_Trigger::CEnvObject_Trigger(const CEnvObject_Trigger& Prototype)
	: CEnvObject(Prototype)
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

	if (m_pCollider && m_pTransformCom)
	{
		m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
#endif
	}
}



void CEnvObject_Trigger::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

HRESULT CEnvObject_Trigger::Ready_TriggerCollider()
{
	const auto AbsAxis = [](_float fValue) -> _float
		{
			return fValue < 0.f ? -fValue : fValue;
		};

	const _float3 vAreaSize =
	{
			AbsAxis(m_tDesc.tEffect.vAreaSize.x),
			AbsAxis(m_tDesc.tEffect.vAreaSize.y),
			AbsAxis(m_tDesc.tEffect.vAreaSize.z)
	};

	constexpr _float kMinTriggerAxis = 0.001f;
	if (vAreaSize.x <= kMinTriggerAxis ||
		vAreaSize.y <= kMinTriggerAxis ||
		vAreaSize.z <= kMinTriggerAxis)
	{
//#ifdef _DEBUG
//		OutputDebugStringA("[EnvTrigger] Skip collider creation due to invalid AreaSize.\n");
//#endif
		return S_OK;
	}

	m_pCollider = Add_Component<CCollider>(
		L"Com_TriggerCollider",
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));

	if (nullptr == m_pCollider)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = m_tDesc.tEffect.vAreaCenter;
	ColliderDesc.vSize = vAreaSize;

	if (FAILED(m_pCollider->Initialize(&ColliderDesc)))
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CEnvObject_Trigger::SetUp_Collider_Callback()
{
	if (m_pCollider)
	{
		m_pCollider->Set_OnEnter([](CCollider* pOther) {
			char szBuf[128];
			sprintf_s(szBuf, "[EnvTrigger] Enter <- group %u\n", pOther->Get_RegisteredGroup());
			OutputDebugStringA(szBuf); 
			});

		m_pCollider->Set_OnStay([](CCollider* pOther) {
			UNREFERENCED_PARAMETER(pOther);
			OutputDebugStringA("[EnvTrigger] Stay\n");
			});

		m_pCollider->Set_OnExit([](CCollider* pOther) {
			char szBuf[128];
			sprintf_s(szBuf, "[EnvTrigger] Exit  <- group %u\n", pOther->Get_RegisteredGroup());
			OutputDebugStringA(szBuf);
			});
	}
}

CEnvObject_Trigger* CEnvObject_Trigger::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvObject_Trigger* pInstance = new CEnvObject_Trigger(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvObject_Trigger");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvObject_Trigger::Clone(void* pArg)
{
	CEnvObject_Trigger* pInstance = new CEnvObject_Trigger(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvObject_Trigger");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Trigger::Free()
{
	__super::Free();
}

NS_END
