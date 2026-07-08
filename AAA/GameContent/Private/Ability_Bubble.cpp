#include "Ability_Bubble.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "Ability_Model.h"

CAbility_Bubble::CAbility_Bubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CContainerObject{ pDevice, pContext }
{
}

CAbility_Bubble::CAbility_Bubble(const CAbility_Bubble& Prototype)
	: CContainerObject (Prototype)
{
}

HRESULT CAbility_Bubble::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CAbility_Bubble::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (pArg)
	{
		auto pDesc = static_cast<const ABILITY_BUBBLE_DESC*>(pArg);
		m_eAbility = pDesc->eAbility;
		m_szModelProtoTag = pDesc->szModelProtoTag;
	}

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	return S_OK;
}

void CAbility_Bubble::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_pCollider && m_pCollider->Is_Enabled())
	{
		m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
#endif
	}
}

HRESULT CAbility_Bubble::Ready_PartObjects()
{
	CAbility_Model::ABILITY_MODEL_DESC desc{};
	desc.pParentMatrix		= m_pTransformCom->Get_WorldMatrixPtr();
	desc.eAbility			= m_eAbility;
	desc.szModelProtoTag	= m_szModelProtoTag;

	if (FAILED(Add_PartObject(m_iPrototypeLevel, CAbility_Model::PROTOTYPE_TAG, PART_MODEL_TAG, &desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CAbility_Bubble::Ready_Collider()
{
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.85f, 0.f };
	ColliderDesc.fRadius = 0.85f;   // Æ©´×°ª

	m_pCollider = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_BubbleCollider"), &ColliderDesc);
	if (nullptr == m_pCollider)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, ETOUI(COLLISION_LAYER::ABILITY_BUBBLE));

	return S_OK;
}

void CAbility_Bubble::Free()
{
	__super::Free();
}
