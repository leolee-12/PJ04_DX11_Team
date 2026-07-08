#include "LD_PopFlower.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"
#include "GameContrnt_Events.h"
#include "Kirby.h"

#include "GameInstance.h"

namespace
{
	inline constexpr const _char* POPFLOWER_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/PopFlower/PopFlower.ysh";
	inline constexpr const _char* POPFLOWER_ANIM_WAIT = "Wait";
	inline constexpr const _char* POPFLOWER_ANIM_OPEN = "Open";
	inline constexpr const _char* POPFLOWER_ANIM_OPEN_WAIT = "OpenWait";
	inline constexpr _uint POPFLOWER_ANIM_PASS = 1u;
	inline constexpr _float POPFLOWER_DEFAULT_ANIM_SPEED = 1.f;
	inline constexpr _float POPFLOWER_COLLISION_ANIM_SPEED = 7.f;

	inline constexpr const _char* POPFLOWER_BLOOM_MESH_NAME = "Flower__PopFlowerC";
	inline constexpr const _char* POPFLOWER_BUD_MESH_NAME = "Tsubomi__PopFlowerC";

	inline constexpr _float POPFLOWER_FOOD_CHANCE_PERCENT = 20.f;
	inline constexpr _uint POPFLOWER_POINT_AMOUNT = 1u;
	inline constexpr _float POPFLOWER_HEAL_AMOUNT = 10.f;
}

NS_BEGIN(Client)

CLD_PopFlower::CLD_PopFlower(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_PopFlower::CLD_PopFlower(const CLD_PopFlower& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tPopFlowerDesc(Prototype.m_tPopFlowerDesc)
	, m_eState(Prototype.m_eState)
	, m_iBudMeshIndex(Prototype.m_iBudMeshIndex)
	, m_iBloomMeshIndex(Prototype.m_iBloomMeshIndex)
{
}

HRESULT CLD_PopFlower::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_PopFlower::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tPopFlowerDesc = *static_cast<const LD_EVENTOBJECT_DESC*>(pArg);
	m_eState = STATE::IDLE;
	m_bInteractionTriggerRegistered = false;
	m_bPlayerOverlapping = false;
	m_iBudMeshIndex = -1;
	m_iBloomMeshIndex = -1;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_PopFlower::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tPopFlowerDesc.strObjectName.c_str()))
		return E_FAIL;

	if (LD_CATEGORY::FOLIAGE != m_tPopFlowerDesc.eCategory)
		return E_FAIL;

	if (MODEL::ANIM != m_tPopFlowerDesc.eModelType || m_tPopFlowerDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
		return E_FAIL;

	if (m_tPopFlowerDesc.bUseCollMesh || !m_tPopFlowerDesc.strAnimEventFile.empty())
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom || nullptr == m_pAnimatorCom || nullptr == m_pInteractionTrigger)
		return E_FAIL;

	if (!m_bInteractionTriggerRegistered)
		return E_FAIL;

	if (m_pModelCom->Get_AnimationIndex(POPFLOWER_ANIM_WAIT) < 0)
		return E_FAIL;

	if (m_pModelCom->Get_AnimationIndex(POPFLOWER_ANIM_OPEN) < 0)
		return E_FAIL;

	if (m_pModelCom->Get_AnimationIndex(POPFLOWER_ANIM_OPEN_WAIT) < 0)
		return E_FAIL;

	if (m_iBudMeshIndex < 0 || m_iBloomMeshIndex < 0 || m_iBudMeshIndex == m_iBloomMeshIndex)
		return E_FAIL;

	return S_OK;
}

void CLD_PopFlower::Update(_float fTimeDelta)
{
	if (!m_bActive || Is_Dead())
		return;

	Update_AnimationSpeed();

	m_pAnimatorCom->Update(fTimeDelta);

	if (STATE::BLOOMING == m_eState && m_pAnimatorCom->Is_Finished())
	{
		if (!Play_Animation(POPFLOWER_ANIM_OPEN_WAIT, true))
			return;

		m_eState = STATE::BLOOMED;
	}
}

void CLD_PopFlower::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bActive || Is_Dead())
		return;

	if (m_pInteractionTrigger->Is_Enabled())
	{
		m_pInteractionTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pInteractionTrigger);
#endif
	}

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLD_PopFlower::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

void CLD_PopFlower::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_PopFlower::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::FOLIAGE;
	Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
	Spec.eModelType = MODEL::ANIM;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	Spec.ModelRequirements =
	{
			{ MODEL_PROTO_TAG, POPFLOWER_MODEL_PATH, MODEL::ANIM, false }
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_PopFlower::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec,
	LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (LD_CATEGORY::FOLIAGE != Spec.eCategory || MODEL::ANIM != Spec.eModelType || Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.bUseCollMesh = false;
	Desc.strAnimEventFile.clear();

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_PopFlower::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_PopFlower::Create(pDevice, pContext);
}

HRESULT CLD_PopFlower::Ready_Components()
{
	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_InitialState()))
		return E_FAIL;

	if (FAILED(Ready_InteractionTrigger()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_PopFlower::Ready_RenderComponents()
{
	m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tPopFlowerDesc.iModelProtoLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC AnimDesc{};
	AnimDesc.pModel = m_pModelCom;

	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_PopFlower::Ready_InitialState()
{
	if (nullptr == m_pModelCom || nullptr == m_pAnimatorCom)
		return E_FAIL;

	m_iBudMeshIndex = Find_MeshIndex_ByName(POPFLOWER_BUD_MESH_NAME);
	m_iBloomMeshIndex = Find_MeshIndex_ByName(POPFLOWER_BLOOM_MESH_NAME);

	if (m_iBudMeshIndex < 0 || m_iBloomMeshIndex < 0 || m_iBudMeshIndex == m_iBloomMeshIndex)
		return E_FAIL;

	if (!Play_Animation(POPFLOWER_ANIM_WAIT, true))
		return E_FAIL;

	m_pAnimatorCom->Seek(0.f);
	m_eState = STATE::IDLE;

	return S_OK;
}

HRESULT CLD_PopFlower::Ready_InteractionTrigger()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pModelCom)
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vCenter =
	{
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f
	};

	const _float3 vHalfExtents =
	{
			(vMax.x - vMin.x) * 0.5f,
			(vMax.y - vMin.y) * 0.5f,
			(vMax.z - vMin.z) * 0.5f
	};

	const _float fRadius = XMVectorGetX(XMVector3Length(XMVectorSet(vHalfExtents.x, vHalfExtents.y, vHalfExtents.z, 0.f)));
	if (fRadius <= 0.f)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.fRadius = fRadius;

	m_pInteractionTrigger = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
		TEXT("Com_InteractionTrigger"), &ColliderDesc);
	if (nullptr == m_pInteractionTrigger)
		return E_FAIL;

	SetUp_InteractionTriggerCallback();

	m_pGameInstance_Proxy->Register_Collider(m_pInteractionTrigger, ETOUI(COLLISION_LAYER::ENV_FOLIAGE));
	m_bInteractionTriggerRegistered = true;

	return S_OK;
}

HRESULT CLD_PopFlower::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_PopFlower::Render_Model()
{
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (!Should_RenderMesh(i))
			continue;

		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind) -> HRESULT
			{
				const _uint iLayerIndex = MTEX_TYPE::UNKNOWN == eType ? 3u : Layer.idx[ETOUI(eType)];
				const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(i, eType);

				if (0u < iTextureCount)
				{
					const _uint iSafeIndex = iLayerIndex < iTextureCount ? iLayerIndex : iTextureCount - 1u;

					if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pConstantName, i, eType, iSafeIndex)))
						return S_OK;
				}

				return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pConstantName, eDefaultKind);
			};

		if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))
			return E_FAIL;

		if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))
			return E_FAIL;

		if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))
			return E_FAIL;

		if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(POPFLOWER_ANIM_PASS)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CLD_PopFlower::SetUp_InteractionTriggerCallback()
{
	if (nullptr == m_pInteractionTrigger)
		return;

	m_pInteractionTrigger->Set_OnEnter([this](CCollider* pOther) { Handle_InteractionTrigger(pOther); });
	m_pInteractionTrigger->Set_OnStay([this](CCollider* pOther) { Handle_InteractionTriggerStay(pOther); });
	m_pInteractionTrigger->Set_OnExit([this](CCollider* pOther) { Handle_InteractionTriggerExit(pOther); });
}

void CLD_PopFlower::Handle_InteractionTrigger(CCollider* pOther)
{
	if (!Is_PlayerCollider(pOther))
		return;

	m_bPlayerOverlapping = true;

	if (STATE::IDLE != m_eState || Is_Dead())
		return;

	if (!Play_Animation(POPFLOWER_ANIM_OPEN, false))
		return;

	m_eState = STATE::BLOOMING;
	Grant_Reward(pOther);
}

void CLD_PopFlower::Handle_InteractionTriggerStay(CCollider* pOther)
{
	if (!Is_PlayerCollider(pOther))
		return;

	m_bPlayerOverlapping = true;
}

void CLD_PopFlower::Handle_InteractionTriggerExit(CCollider* pOther)
{
	if (!Is_PlayerCollider(pOther))
		return;

	m_bPlayerOverlapping = false;
}

void CLD_PopFlower::Grant_Reward(CCollider* pOther)
{
	if (nullptr == pOther)
		return;

	CKirby* pKirby = dynamic_cast<CKirby*>(pOther->Get_Owner());
	if (nullptr == pKirby)
		return;

	const _float fRewardRoll = m_pGameInstance_Proxy->RandomFloat(0.f, 100.f);
	if (fRewardRoll < POPFLOWER_FOOD_CHANCE_PERCENT)
	{
		pKirby->Add_HP(POPFLOWER_HEAL_AMOUNT);
		return;
	}

	KIRBY_POINTSTAR_GAINED_DESC Desc{};
	Desc.iAmount = POPFLOWER_POINT_AMOUNT;
	m_pGameInstance_Proxy->Publish(EventTag::Kirby_PointStarGained, &Desc);
}

void CLD_PopFlower::Unregister_InteractionTrigger(_bool bImmediate)
{
	if (nullptr == m_pInteractionTrigger)
		return;

	m_pInteractionTrigger->Set_Enabled(false);

	if (!m_bInteractionTriggerRegistered)
		return;

	const _uint iGroup = ETOUI(COLLISION_LAYER::ENV_TRIGGER);

	if (bImmediate)
		m_pGameInstance_Proxy->Immediate_Unregister(m_pInteractionTrigger, iGroup);
	else
		m_pGameInstance_Proxy->Request_Unregister(m_pInteractionTrigger, iGroup);

	m_bInteractionTriggerRegistered = false;
}

_bool CLD_PopFlower::Play_Animation(const _char* pAnimName, _bool bLoop)
{
	if (m_pModelCom->Get_AnimationIndex(pAnimName) < 0)
		return false;

	m_pAnimatorCom->Resume();
	m_pAnimatorCom->Play(pAnimName, bLoop, true, 0.f, POPFLOWER_DEFAULT_ANIM_SPEED);
	Update_AnimationSpeed();

	return true;
}

_bool CLD_PopFlower::Is_PlayerCollider(CCollider* pOther) const
{
	if (nullptr == pOther)
		return false;

	if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
		return false;

	return nullptr != dynamic_cast<CKirby*>(pOther->Get_Owner());
}

void CLD_PopFlower::Update_AnimationSpeed()
{
	const _float fSpeed = m_bPlayerOverlapping ? POPFLOWER_COLLISION_ANIM_SPEED : POPFLOWER_DEFAULT_ANIM_SPEED;
	m_pAnimatorCom->Set_PlaySpeed(fSpeed);
}

_int CLD_PopFlower::Find_MeshIndex_ByName(const _char* pMeshName) const
{
	if (nullptr == m_pModelCom || nullptr == pMeshName)
		return -1;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (m_pModelCom->Get_MeshName(i) == pMeshName)
			return static_cast<_int>(i);
	}

	return -1;
}

_bool CLD_PopFlower::Should_RenderMesh(_uint iMeshIndex) const
{
	const _int iMesh = static_cast<_int>(iMeshIndex);

	if (iMesh == m_iBudMeshIndex)
		return STATE::IDLE == m_eState;

	if (iMesh == m_iBloomMeshIndex)
		return STATE::IDLE != m_eState;

	return true;
}

CLD_PopFlower* CLD_PopFlower::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_PopFlower* pInstance = new CLD_PopFlower(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_PopFlower");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_PopFlower::Clone(void* pArg)
{
	CLD_PopFlower* pInstance = new CLD_PopFlower(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_PopFlower");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_PopFlower::Free()
{
	Unregister_InteractionTrigger(true);

	__super::Free();
}

NS_END