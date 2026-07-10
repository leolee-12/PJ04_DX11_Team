#include "Ability_Bubble.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Bubble_Manager.h"

#include "Ability_Model.h"
#include "Effect_Loader.h"
#include "Effect_Container.h";


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

	m_vBaseScale = m_pTransformCom->Get_Scaled();

	return S_OK;
}

void CAbility_Bubble::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);

	if (m_pCollider && m_pCollider->Is_Enabled())
	{
		m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
#endif
	}
}

void CAbility_Bubble::Set_RenderActive(_bool bActive)
{
	if (m_pModelPart)
		m_pModelPart->Set_ModelRenderActive(bActive);

	if (bActive)
		Start_Aura();
	else 
		Stop_Aura();

}

void CAbility_Bubble::Set_Pool(CBubble_Manager* pPool, _uint iLevel, const _wstring& strKey)
{
	m_pPool = pPool;
	m_iPoolLevel = iLevel;
	m_strPoolKey = strKey;
}

void CAbility_Bubble::Activate(const _float3& vPos)
{
	m_bActive = true;
	m_bAvailable = true;
	m_fTimer = 0.f;

	m_pTransformCom->Set_State(STATE::POSITION,	XMVectorSetW(XMLoadFloat3(&vPos), 1.f));
	m_pTransformCom->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);

	Set_RenderActive(true);

	if (m_pCollider)
		m_pCollider->Set_Enabled(true);
}

void CAbility_Bubble::Start_Aura()
{
	if (m_pAura)
		return;

	CEffect_Loader::GetInstance()->Spawn(AURA_EFFECT_ID, Get_LevelIndex(),
											_float3(0.f, -0.45f, 0.f),
											_float3(0.f, 0.f, 0.f),
											_float3(0.f, 0.f, 0.f),
											m_pTransformCom->Get_WorldMatrixPtr(),
											&m_pAura);
}

void CAbility_Bubble::Stop_Aura()
{
	if (nullptr == m_pAura)
		return;

	m_pAura->EffectContainer_StopAfterEmission();
	m_pAura = nullptr;
}

HRESULT CAbility_Bubble::Ready_PartObjects()
{
	CAbility_Model::ABILITY_MODEL_DESC desc{};
	desc.pParentMatrix		= m_pTransformCom->Get_WorldMatrixPtr();
	desc.eAbility			= m_eAbility;
	desc.szModelProtoTag	= m_szModelProtoTag;

	if (FAILED(Add_PartObject(m_iPrototypeLevel, CAbility_Model::PROTOTYPE_TAG, PART_MODEL_TAG, &desc)))
		return E_FAIL;

	CAbility_Model* pInstance = dynamic_cast<CAbility_Model*>(m_PartObjects[PART_MODEL_TAG]);
	if (pInstance == nullptr)
		return E_FAIL;

	m_pModelPart = pInstance;

	return S_OK;
}

HRESULT CAbility_Bubble::Ready_Collider()
{
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
/*	ColliderDesc.vCenter = { 0.f, 0.85f, 0.f };
	ColliderDesc.fRadius = 0.85f;*/		// 이펙트 사이즈는 이거로 진행 ( 배치 위치랑 크기 감 잡기 용도로 남겨둠

	ColliderDesc.vCenter = { 0.f, 0.85f, 0.f };
	ColliderDesc.fRadius = 1.25f;		// 충돌체 사이즈는 이거로 고정

	m_pCollider = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_BubbleCollider"), &ColliderDesc);
	if (nullptr == m_pCollider)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, ETOUI(m_eCollLayer));

	return S_OK;
}

void CAbility_Bubble::Return_ToPool()
{
	Stop_Aura();

	if (m_pPool)
		m_pPool->Return(m_iPoolLevel, m_strPoolKey, this);
}

void CAbility_Bubble::Free()
{
	//Stop_Aura();

	__super::Free();
}
