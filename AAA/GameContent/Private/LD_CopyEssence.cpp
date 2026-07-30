#include "LD_CopyEssence.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"
#include "EssenceBubble.h"
#include "Bubble_Manager.h"
#include "Effect_Loader.h"

#include "GameInstance.h"

namespace
{
	inline constexpr const _char* COPYESSENCE_MODEL_PATH =	"../../Resources/CHJ/Gimmick/CopyEssence/CopyEssence.ysh";

	inline constexpr const _char* ANIM_APPEAR = "Appear";
	inline constexpr const _char* ANIM_COPYENABLE = "CopyEnable";

	inline constexpr const _char* PATH_ABILITYKIND ="Gimmick.CopyEssence.MainComponent.AbilityKind";
	inline constexpr const _char* PATH_ISACTITEM =	"Gimmick.CopyEssence.MainComponent.IsActAbilityItem";

	inline constexpr _float COPYESSENCE_ANIM_SPEED = 1.f;
	inline constexpr _uint  COPYESSENCE_ANIM_PASS = 1u;

	inline constexpr const _char* SOCKET_BONE = "RotL";
	inline constexpr const _tchar* BUBBLE_LAYER_TAG = L"Layer_AbilityBubble";

	inline constexpr const _tchar* MODEL_TAG_SWORD = L"Prototype_Component_Model_Sword";
	inline constexpr const _tchar* MODEL_TAG_BOMB = L"Prototype_Component_Model_KirbyBomb";
	inline constexpr const _tchar* MODEL_TAG_ICE = L"Prototype_Component_Model_IceHat";
}

NS_BEGIN(Client)

CLD_CopyEssence::CLD_CopyEssence(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject{ pDevice, pContext }
{
}

CLD_CopyEssence::CLD_CopyEssence(const CLD_CopyEssence& Prototype)
	: CLevelDesignObject (Prototype)
	, m_eState(Prototype.m_eState)
	, m_eAbility(Prototype.m_eAbility)
{
}

HRESULT CLD_CopyEssence::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_CopyEssence::Initialize(void* pArg)
{
	if (pArg == nullptr)
		return E_FAIL;

	m_tCopyEssenceDesc = *static_cast<const LD_EVENTOBJECT_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_RigidStatic()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	_wstring strKind;
	JsonUtils::Try_ReadString(m_tCopyEssenceDesc.jRaw, PATH_ABILITYKIND, &strKind);
	Apply_AbilityKind(strKind);

	_bool bActAbilityItem = false;
	JsonUtils::Try_ReadBoolFromNumeric(m_tCopyEssenceDesc.jRaw, PATH_ISACTITEM, &bActAbilityItem);

	if (bActAbilityItem)
	{
		m_eState = STATE::APPEAR;
		m_pAnimatorCom->Play(ANIM_APPEAR, false, true, 0.f, COPYESSENCE_ANIM_SPEED);
	}
	else
	{
		m_eState = STATE::ENABLE;
		m_pAnimatorCom->Play(ANIM_COPYENABLE, true, true, 0.f, COPYESSENCE_ANIM_SPEED);
	}

	return S_OK;
}

HRESULT CLD_CopyEssence::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(
		OBJECT_NAME, m_tCopyEssenceDesc.strObjectName.c_str()))
		return E_FAIL;

	if (LD_CATEGORY::GIMMICK != m_tCopyEssenceDesc.eCategory)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom
		|| nullptr == m_pAnimatorCom)
		return E_FAIL;

	if (1u > m_pModelCom->Get_NumMeshes())
		return E_FAIL;

	if (m_pModelCom->Get_AnimationIndex(ANIM_APPEAR) < 0)
		return E_FAIL;

	if (m_pModelCom->Get_AnimationIndex(ANIM_COPYENABLE) < 0)
		return E_FAIL;

	if (nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLD_CopyEssence::Update(_float fTimeDelta)
{
	if (!m_bActive || Is_Dead())
		return;

	m_pAnimatorCom->Update(fTimeDelta);

	if (m_pAnimatorCom->Is_Finished() && STATE::APPEAR == m_eState)
	{
		m_eState = STATE::ENABLE;
		m_pAnimatorCom->Play(
			ANIM_COPYENABLE, true, true, 0.f, COPYESSENCE_ANIM_SPEED);
	}

	if (STATE::ENABLE == m_eState && !m_bBubbleSpawned)
	{
		m_bBubbleSpawned = true;
		Spawn_AbilityBubble();
	}
}

void CLD_CopyEssence::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bActive || Is_Dead())
		return;

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLD_CopyEssence::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

void CLD_CopyEssence::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_CopyEssence::Register_LevelDesignSpecs()
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
	Spec.ModelRequirements =
	{
		{ MODEL_PROTO_TAG, COPYESSENCE_MODEL_PATH,
		  MODEL::ANIM, true },
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_CopyEssence::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(
		OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG
		|| Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_CopyEssence::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_CopyEssence::Create(pDevice, pContext);
}

HRESULT CLD_CopyEssence::Ready_RenderComponents()
{
	m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID,	Shader_AnimMesh_PBR.szProtoTag,	TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tCopyEssenceDesc.iModelProtoLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC AnimDesc{};
	AnimDesc.pModel = m_pModelCom;

	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"),	CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_CopyEssence::Ready_RigidStatic()
{
	Release_RigidStatic();

	if (nullptr == m_pGameInstance_Proxy
		|| nullptr == m_pTransformCom
		|| nullptr == m_pModelCom)
		return E_FAIL;

	auto pCollisionMesh = m_pModelCom->Get_CollisionMesh();
	if (nullptr == pCollisionMesh)
		return E_FAIL;

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(
		pCollisionMesh,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	if (nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLD_CopyEssence::Release_RigidStatic()
{
	if (nullptr == m_pRigidStatic)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);

	m_pRigidStatic = nullptr;
}

HRESULT CLD_CopyEssence::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
		m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix",
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix",
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
	return m_pShaderCom->Bind_RawValue(
		"g_vEmissiveColor", &vEmissiveColor, sizeof(_float4));
}

HRESULT CLD_CopyEssence::Render_Model()
{
	auto BindTex = [&](
		const _char* pName, MTEX_TYPE eType, DEFAULT_TEXTURE eKind)
		-> HRESULT
		{
			if (0u < m_pModelCom->Get_MeshTextureCount(0u, eType))
			{
				if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pName, 0u, eType, 0u)))
					return S_OK;
			}

			return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(
				m_pShaderCom, pName, eKind);
		};

	if (FAILED(BindTex("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))
		return E_FAIL;

	if (FAILED(BindTex("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))
		return E_FAIL;

	if (FAILED(BindTex("g_MRATexture",	MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))
		return E_FAIL;

	if (FAILED(BindTex("g_UnknownTexture",	MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", 0u)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(COPYESSENCE_ANIM_PASS)))
		return E_FAIL;

	return m_pModelCom->Render(0u);
}

void CLD_CopyEssence::Apply_AbilityKind(const _wstring& strKind)
{
	if (strKind == L"Sword")		m_eAbility = COPY_ABILITY_TYPE::SWORD;
	else if (strKind == L"Bomb")	m_eAbility = COPY_ABILITY_TYPE::BOMB;
	else if (strKind == L"Ice")		m_eAbility = COPY_ABILITY_TYPE::ICE;
	else if (strKind == L"Hammer")	m_eAbility = COPY_ABILITY_TYPE::TOY_HAMMER;
	else if (strKind == L"Crash")	m_eAbility = COPY_ABILITY_TYPE::CRASH;
	else							m_eAbility = COPY_ABILITY_TYPE::NORMAL;
}

void CLD_CopyEssence::Spawn_AbilityBubble()
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	const _float4x4* pSocket = m_pModelCom->Get_BoneMatrixPtr(SOCKET_BONE);
	if (nullptr == pSocket)
		return;

	_matrix matWorld	= XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	_matrix matSocket	=	XMLoadFloat4x4(pSocket) * matWorld;

	_float3 vPos{};
	XMStoreFloat3(&vPos, matSocket.r[3]);

	CEffect_Loader::GetInstance()->Spawn(
		TEXT("EssenceAura"), m_iLevelIndex, vPos);

	auto* pMgr = CBubble_Manager::GetInstance();
	auto eKind = CBubble_Manager::BUBBLE_KIND::ESSENCE;

	CAbility_Bubble* pBubble = nullptr;
	if (FAILED(pMgr->Spawn(m_iLevelIndex, eKind, m_eAbility, vPos, &pBubble)))
		return;

	m_pBubble =	dynamic_cast<CEssenceBubble*>(pBubble);
}

CLD_CopyEssence* CLD_CopyEssence::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_CopyEssence* pInstance = new CLD_CopyEssence(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_CopyEssence");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_CopyEssence::Clone(void* pArg)
{
	CLD_CopyEssence* pInstance = new CLD_CopyEssence(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_CopyEssence");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_CopyEssence::Free()
{
	Release_RigidStatic();

	__super::Free();
}

NS_END