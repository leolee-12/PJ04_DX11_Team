#include "LD_SlopeBoardC.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "GameInstance.h"

namespace
{
	inline constexpr const _char* SLOPEBOARD_C_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardC.ysh";
	inline constexpr const _char * SLOPEBOARD_C_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { "FallenWait", "Wait", "Cut1", "" };
}

NS_BEGIN(Client)

CLD_SlopeBoardC::CLD_SlopeBoardC(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_SlopeBoardC::CLD_SlopeBoardC(const CLD_SlopeBoardC& Prototype)
	: CLD_EventObject(Prototype)
	, m_eState(Prototype.m_eState)
	, m_fEventAnimSpeed(Prototype.m_fEventAnimSpeed)
{
}

HRESULT CLD_SlopeBoardC::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tEventObjectDesc.strObjectName.c_str()))
		return E_FAIL;

	if (m_tEventObjectDesc.eModelType != MODEL::ANIM || m_tEventObjectDesc.wstrModelProtoTag !=
		MODEL_PROTO_TAG)
		return E_FAIL;

	if (!m_tEventObjectDesc.bUseCollMesh || !m_tEventObjectDesc.strAnimEventFile.empty())
		return E_FAIL;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
	{
		if (m_tEventObjectDesc.strAnimNames[i] != SLOPEBOARD_C_ANIM_NAMES[i])
			return E_FAIL;
	}

	return S_OK;
}

void CLD_SlopeBoardC::Update(_float fTimeDelta)
{
	const _bool bAnimationWasActive = m_bAnimationActive;

	__super::Update(fTimeDelta);

	if (bAnimationWasActive && !m_bAnimationActive && STATE::PLAYING == m_eState)
		m_eState = STATE::PLAYED;
}

void CLD_SlopeBoardC::Late_Update(_float fTimeDelta)
{
	if (nullptr != m_pInteractionCollider && m_pInteractionCollider->Is_Enabled())
	{
		m_pInteractionCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pInteractionCollider);
#endif
	}

	__super::Late_Update(fTimeDelta);
}

void CLD_SlopeBoardC::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_SlopeBoardC::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::GIMMICK;
	Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
	Spec.eModelType = MODEL::ANIM;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	Spec.ModelRequirements = { { MODEL_PROTO_TAG, SLOPEBOARD_C_MODEL_PATH, MODEL::ANIM, true }, };

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_SlopeBoardC::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const
	LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK || Spec.eModelType != MODEL::ANIM ||
		Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.bUseCollMesh = true;
	Desc.strAnimEventFile.clear();

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
		Desc.strAnimNames[i] = SLOPEBOARD_C_ANIM_NAMES[i];

	*pOutEntry = Desc;
	return true;
}

HRESULT CLD_SlopeBoardC::Ready_Components()
{
	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_AnimEvents()))
		return E_FAIL;

	m_MeshVisible.assign(static_cast<size_t>(m_pModelCom->Get_NumMeshes()), true);

	if (FAILED(Ready_RigidStatic()))
		return E_FAIL;

	if (FAILED(Ready_InteractionCollider()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_SlopeBoardC::Ready_RenderComponents()
{
	if (FAILED(__super::Ready_RenderComponents()))
		return E_FAIL;

	m_eState = STATE::IDLE;

	const _int iAnimationIndex = m_pModelCom->Get_AnimationIndex(SLOPEBOARD_C_ANIM_NAMES[2u]);
	if (iAnimationIndex < 0)
		return E_FAIL;

	m_pModelCom->Set_AnimationIndex(static_cast<_uint>(iAnimationIndex), false, true, 0.f);
	m_pModelCom->Seek_Animation(0.f);
	m_bAnimationActive = false;

	return S_OK;
}

HRESULT CLD_SlopeBoardC::Ready_InteractionCollider()
{
	_float3 vMin{}, vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	const _float3 vSize = { vMax.x - vMin.x, vMax.y - vMin.y, vMax.z - vMin.z };

	if (vSize.x <= 0.f || vSize.y <= 0.f || vSize.z <= 0.f)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.vSize = vSize;

	m_pInteractionCollider = Add_Component<CCollider>(Collider_OBB.iLevelID, Collider_OBB.szProtoTag, TEXT("Com_InteractionCollider"),
		&ColliderDesc);
	if (nullptr == m_pInteractionCollider)
		return E_FAIL;

	m_pInteractionCollider->Set_OnEnter([this](CCollider* pOther) { Handle_Interaction(pOther); });
	m_pGameInstance_Proxy->Register_Collider(m_pInteractionCollider, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CLD_SlopeBoardC::Handle_Interaction(CCollider* pOther)
{
	if (nullptr == pOther)
		return;

	if (ETOUI(COLLISION_LAYER::CAR_BOOST) != pOther->Get_RegisteredGroup())
		return;

	if (STATE::IDLE != m_eState)
		return;

	m_pAnimatorCom->Play(m_tEventObjectDesc.strAnimNames[2], false, true, 0.f, m_fEventAnimSpeed);
	m_bAnimationActive = true;
	m_eState = STATE::PLAYING;

	m_pInteractionCollider->Set_Enabled(false);
}

CGameObject* CLD_SlopeBoardC::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_SlopeBoardC::Create(pDevice, pContext);
}

CLD_SlopeBoardC* CLD_SlopeBoardC::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_SlopeBoardC* pInstance = new CLD_SlopeBoardC(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_SlopeBoardC");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_SlopeBoardC::Clone(void* pArg)
{
	CLD_SlopeBoardC* pInstance = new CLD_SlopeBoardC(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_SlopeBoardC");
		Safe_Release(pInstance);
	}

	return pInstance;
}

NS_END