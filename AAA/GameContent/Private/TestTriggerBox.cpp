#include "TestTriggerBox.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CTestTriggerBox::CTestTriggerBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
	, m_vTriggerCenterLocal{ 0.f, 0.f, 0.f }
	, m_vTriggerSize{ 4.f, 4.f, 4.f }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CTestTriggerBox::CTestTriggerBox(const CTestTriggerBox& Prototype)
	: CEnvObject(Prototype)
	, m_strTriggerId(Prototype.m_strTriggerId)
	, m_vTriggerCenterLocal(Prototype.m_vTriggerCenterLocal)
	, m_vTriggerSize(Prototype.m_vTriggerSize)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CTestTriggerBox::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CTestTriggerBox::Initialize(void* pArg)
{
	ENV_OBJECT_DESC Desc{};

	if (pArg != nullptr)
		Desc = *static_cast<const ENV_OBJECT_DESC*>(pArg);
	else
		Build_DefaultDesc(&Desc);

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_TriggerCollider()))
		return E_FAIL;

	return S_OK;
}

void CTestTriggerBox::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_pCollider && m_pTransformCom)
	{
		m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
#endif
	}
}

void CTestTriggerBox::Enter_Collision(CGameObject* pOther)
{
	UNREFERENCED_PARAMETER(pOther);

#ifdef _DEBUG
	OutputDebugStringA("[EnvTrigger] Enter_Collision\n");
#endif
}

void CTestTriggerBox::On_Collision(CGameObject* pOther)
{
	UNREFERENCED_PARAMETER(pOther);

#ifdef _DEBUG
	OutputDebugStringA("[EnvTrigger] On_Collision\n");
#endif
}

void CTestTriggerBox::Exit_Collision(CGameObject* pOther)
{
	UNREFERENCED_PARAMETER(pOther);

#ifdef _DEBUG
	OutputDebugStringA("[EnvTrigger] Exit_Collision\n");
#endif
}

HRESULT CTestTriggerBox::Ready_TriggerCollider()
{
	if (m_vTriggerSize.x <= 0.001f ||
		m_vTriggerSize.y <= 0.001f ||
		m_vTriggerSize.z <= 0.001f)
		return S_OK;

	m_pCollider = Add_Component<CCollider>(
		L"Com_TriggerCollider",
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));
	if (nullptr == m_pCollider)
		return E_FAIL;

	CCollider::COLLIDER_DESC Desc{};
	Desc.pOwner = this;
	Desc.vCenter = m_vTriggerCenterLocal;
	Desc.vSize = m_vTriggerSize;

	if (FAILED(m_pCollider->Initialize(&Desc)))
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, CL_ENV_TRIGGER);
	return S_OK;
}

void CTestTriggerBox::Build_DefaultDesc(ENV_OBJECT_DESC* pOutDesc)
{
	pOutDesc->eKind = ENV_OBJECT_KIND::EFFECT;
	pOutDesc->wstrObjectName = L"TestEnvTrigger";
	pOutDesc->wstrComponentName = L"TestEnvTrigger";
	pOutDesc->vPosition = { 0.f, 0.f, 0.f };
	pOutDesc->vRotation = { 0.f, 0.f, 0.f, 1.f };
	pOutDesc->vScale = { 1.f, 1.f, 1.f };
	pOutDesc->tRender.bUseLodCulling = false;
	pOutDesc->tRender.bShadowMappingCaster = false;
	pOutDesc->tCollision.bInvisibleCollision = true;
}

CTestTriggerBox* CTestTriggerBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTestTriggerBox* pInstance = new CTestTriggerBox(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CTestTriggerBox");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTestTriggerBox::Clone(void* pArg)
{
	CTestTriggerBox* pInstance = new CTestTriggerBox(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CTestTriggerBox");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTestTriggerBox::Free()
{
	__super::Free();
}

NS_END