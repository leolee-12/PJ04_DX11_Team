#include "LevelDesign_Breakable.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"
#include "Effect_Loader.h"

#include "GameInstance.h"

namespace
{
	struct LD_BREAKABLE_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		MODEL eModelType;
		_uint iBreakAnimIndex;
		const _char* pBaseMeshName;
		_bool bCookCollisionMesh;
	};

	static const LD_BREAKABLE_CATALOG g_BreakableCatalog[] =
	{
		{ L"WoodBox", CLevelDesign_Breakable::WOODBOX_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/BoxWood/BoxWood.ysh", MODEL::ANIM, 2u, "WoodBoxM__BoxWoodC", false},
		{ L"BoxPlastic", CLevelDesign_Breakable::PLASTICBOX_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/BoxPlastic/BoxPlastic.ysh", MODEL::ANIM, 0u, "BoxPlasticM__BoxPlasticC", false},
		{ L"BreakableRockS", CLevelDesign_Breakable::BREAKABLE_ROCK_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/BreakableRock/BreakableRock_S.ysh", MODEL::NONANIM, LD_INVALID_ID, nullptr, true },
		{ L"BreakableRockM", CLevelDesign_Breakable::BREAKABLE_ROCK_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/BreakableRock/BreakableRock_M.ysh", MODEL::NONANIM, LD_INVALID_ID, nullptr, true },
		{ L"BreakableRockMForBridge", CLevelDesign_Breakable::BREAKABLE_ROCK_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/BreakableRock/BreakableRock_M.ysh", MODEL::NONANIM, LD_INVALID_ID, nullptr, true }
	};

	static const LD_BREAKABLE_CATALOG* Find_BreakableCatalog(const _wstring& wstrObjName)
	{
		for (const LD_BREAKABLE_CATALOG& Entry : g_BreakableCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, wstrObjName.c_str()))
				return &Entry;
		}

		return nullptr;
	}

	void Build_DefaultBreakableDesc(LD_BREAKABLE_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return;

		*pOutDesc = {};

		pOutDesc->wstrSourcePath = L"Palette";
		pOutDesc->strSourceFile = L"H1W1.ysh";
		pOutDesc->strSection = L"Palette";
		pOutDesc->strEntryKey = L"StarBlock_Default";
		pOutDesc->strObjectName = L"StarBlock";
		pOutDesc->strKind = L"Palette";

		pOutDesc->eCategory = LD_CATEGORY::BREAKABLE;
		pOutDesc->wstrModelProtoTag = CLevelDesign_Breakable::STARBLOCK_H1W1_MODEL_PROTO_TAG;

		pOutDesc->fScale = 1.f;
		pOutDesc->vRight = { 1.f, 0.f, 0.f, 0.f };
		pOutDesc->vUp = { 0.f, 1.f, 0.f, 0.f };
		pOutDesc->vLook = { 0.f, 0.f, 1.f, 0.f };
		pOutDesc->vPosition = { 0.f, 0.f, 0.f, 1.f };

		pOutDesc->vParsedPosition = { 0.f, 0.f, 0.f };
		pOutDesc->qParsedRotation = { 0.f, 0.f, 0.f, 1.f };
		pOutDesc->vParsedScale = { 1.f, 1.f, 1.f };
	}
}

NS_BEGIN(Client)

CLevelDesign_Breakable::CLevelDesign_Breakable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Breakable::CLevelDesign_Breakable(const CLevelDesign_Breakable& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tBreakableDesc(Prototype.m_tBreakableDesc)
{
}

HRESULT CLevelDesign_Breakable::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Breakable::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (nullptr == pArg)
	{
		LD_BREAKABLE_DESC DefaultDesc{};
		Build_DefaultBreakableDesc(&DefaultDesc);
		m_tBreakableDesc = DefaultDesc;
	}
	else
	{
		m_tBreakableDesc = *static_cast<const LD_BREAKABLE_DESC*>(pArg);
		if (FAILED(Validate_Desc()))
			return E_FAIL;
	}

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_HurtBox()))
		return E_FAIL;

	if (FAILED(Ready_PhysicsActor_Box()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Breakable::Update(_float fTimeDelta)
{
	if (MODEL::ANIM == m_tBreakableDesc.eModelType && BREAKABLE_STATE::BREAKING == m_eState)
	{
		m_pAnimatorCom->Update(fTimeDelta);

		if (m_pAnimatorCom->Is_Finished())
			m_eState = BREAKABLE_STATE::DESTROYED;
	}
}

void CLevelDesign_Breakable::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (BREAKABLE_STATE::DESTROYED == m_eState)
		return;

	if (BREAKABLE_STATE::INTACT == m_eState)
	{
		m_pHurtBoxCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBoxCom);
#endif
	}

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Breakable::Render()
{
	if (BREAKABLE_STATE::DESTROYED == m_eState)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (MODEL::ANIM == m_tBreakableDesc.eModelType)
		{
			if (BREAKABLE_STATE::INTACT == m_eState && i != m_iBaseMeshIndex)
				continue;

			if (BREAKABLE_STATE::BREAKING == m_eState && i == m_iBaseMeshIndex)
				continue;
		}

		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		if (MODEL::ANIM == m_tBreakableDesc.eModelType)
		{
			auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind) -> HRESULT
				{
					const _uint iLayerIndex = Layer.idx[ETOUI(eType)];
					const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(i, eType);

					if (0u < iTextureCount)
					{
						const _uint iSafeIndex = (iLayerIndex < iTextureCount) ? iLayerIndex : (iTextureCount - 1u);

						if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pConstantName, i, eType, iSafeIndex)))
							return S_OK;
					}

					return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pConstantName, eDefaultKind);
				};

			if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))		return E_FAIL;
			if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))	return E_FAIL;
			if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))			return E_FAIL;
			if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))		return E_FAIL;

			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;

			const _uint iBoxPass = 5u;

			if (FAILED(m_pShaderCom->Begin(iBoxPass)))
				return E_FAIL;
		}
		else
		{
			MESH_LAYER_BIND_CONTEXT Ctx{};
			Ctx.pShader = m_pShaderCom;
			Ctx.pModel = m_pModelCom;
			Ctx.pGI_Proxy = m_pGameInstance_Proxy;
			Ctx.iMesh = i;
			Ctx.pLayer = &Layer;
			Ctx.eProfile = MESH_LAYER_PROFILE::NONANIM_PBR;
			Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
			Ctx.iFallbackPass = ShaderPass::NonAnimPBR::DMN;
			Ctx.fDissolve = 0.f;

			MESH_LAYER_BIND_RESULT Result{};
			if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
				return E_FAIL;

			if (Result.bSkipMesh)
				continue;

			if (FAILED(m_pShaderCom->Begin(Result.iPass)))
				return E_FAIL;
		}

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Breakable::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLevelDesign_Breakable::Damaged(const ATTACK_INFO& tInfo)
{
	UNREFERENCED_PARAMETER(tInfo);

	if (BREAKABLE_STATE::INTACT != m_eState)
		return;

	const _float4* pCamLook = m_pGameInstance_Proxy->Get_CamLook();

	_float3 vFaceCam{};
	XMStoreFloat3(&vFaceCam, XMVectorNegate(XMLoadFloat4(pCamLook)));

	_float3 vPos{};
	XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));

	CEffect_Loader::GetInstance()->Spawn(L"CommonHit", Get_LevelIndex(), vPos, vFaceCam, _float3(0.f, 0.f, 0.f), nullptr);

	Release_PhysicsActor();
	m_pHurtBoxCom->Set_Enabled(false);

	if (MODEL::ANIM == m_tBreakableDesc.eModelType)
	{
		m_eState = BREAKABLE_STATE::BREAKING;
		m_pAnimatorCom->Set_PlaySpeed(1.5f);
		m_pModelCom->Set_AnimationIndex(m_iBreakAnimIndex, false, true, 0.f);
		return;
	}

	m_eState = BREAKABLE_STATE::DESTROYED;
}

void CLevelDesign_Breakable::Register_LevelDesignSpecs()
{
	for (const LD_BREAKABLE_CATALOG& Entry : g_BreakableCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_Gimmick";
		Spec.eCategory = LD_CATEGORY::BREAKABLE;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.eModelType = Entry.eModelType;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.ModelRequirements =
		{
			{ Entry.pModelProtoTag, Entry.pModelPath, Entry.eModelType, Entry.bCookCollisionMesh }
		};

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLevelDesign_Breakable::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;
	if (nullptr == Find_BreakableCatalog(CommonDesc.strObjectName))
		return false;
	if (Spec.eCategory != LD_CATEGORY::BREAKABLE || Spec.wstrModelProtoTag.empty())
		return false;

	LD_BREAKABLE_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLevelDesign_Breakable::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_Breakable::Create(pDevice, pContext);
}

HRESULT CLevelDesign_Breakable::Validate_Desc()
{
	if (m_tBreakableDesc.eCategory != LD_CATEGORY::BREAKABLE)
		return E_FAIL;
	if (m_tBreakableDesc.wstrModelProtoTag.empty())
		return E_FAIL;
	if (MODEL::ANIM != m_tBreakableDesc.eModelType && MODEL::NONANIM != m_tBreakableDesc.eModelType)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Breakable::Ready_Components()
{
	const LD_BREAKABLE_CATALOG* pCatalog = Find_BreakableCatalog(m_tBreakableDesc.strObjectName);
	if (nullptr == pCatalog || pCatalog->eModelType != m_tBreakableDesc.eModelType)
		return E_FAIL;

	const _tchar* pModelProtoTag = Resolve_ModelProtoTag();
	if (nullptr == pModelProtoTag)
		return E_FAIL;

	const auto& ShaderDesc = MODEL::ANIM == m_tBreakableDesc.eModelType
		? Shader_AnimMesh_PBR
		: Shader_NonAnimMesh_PBR;

	m_pShaderCom = Add_Component<CShader>(ShaderDesc.iLevelID, ShaderDesc.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tBreakableDesc.iModelProtoLevel, pModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	if (MODEL::ANIM == m_tBreakableDesc.eModelType)
	{
		m_iBreakAnimIndex = pCatalog->iBreakAnimIndex;

		const _uint iNumAnimations = m_pModelCom->Get_NumAnimations();
		if (LD_INVALID_ID == m_iBreakAnimIndex || m_iBreakAnimIndex >= iNumAnimations)
			return E_FAIL;
		if (m_pModelCom->Get_AnimationName(m_iBreakAnimIndex) != "Break")
			return E_FAIL;
		if (nullptr == pCatalog->pBaseMeshName)
			return E_FAIL;

		const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
		for (_uint i = 0; i < iNumMeshes; ++i)
		{
			if (m_pModelCom->Get_MeshName(i) == pCatalog->pBaseMeshName)
			{
				m_iBaseMeshIndex = i;
				break;
			}
		}

		if (LD_INVALID_ID == m_iBaseMeshIndex)
			return E_FAIL;

		CAnimator::ANIMATOR_DESC AnimDesc{};
		AnimDesc.pModel = m_pModelCom;

		m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
		if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
			return E_FAIL;

		m_pModelCom->Update_Combined();
	}

	return S_OK;
}

HRESULT CLevelDesign_Breakable::Ready_HurtBox()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pModelCom)
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vSize = { vMax.x - vMin.x, vMax.y - vMin.y, vMax.z - vMin.z };

	_float fRadius = vSize.x;
	if (fRadius < vSize.y)
		fRadius = vSize.y;
	if (fRadius < vSize.z)
		fRadius = vSize.z;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	ColliderDesc.fRadius = fRadius * 0.5f;

	m_pHurtBoxCom = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_HurtBox"),
		&ColliderDesc);
	if (nullptr == m_pHurtBoxCom)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBoxCom, ETOUI(COLLISION_LAYER::ENV_HURT));

	return S_OK;
}

HRESULT CLevelDesign_Breakable::Ready_PhysicsActor_Box()
{
	Release_PhysicsActor();

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pModelCom)
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vLocalCenter = {
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f
	};

	const _float3 vLocalHalfExtents = {
			(vMax.x - vMin.x) * 0.5f,
			(vMax.y - vMin.y) * 0.5f,
			(vMax.z - vMin.z) * 0.5f
	};

	m_pPhysicsActor = m_pGameInstance_Proxy->Create_StaticBox(
		vLocalCenter,
		vLocalHalfExtents,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	if (nullptr == m_pPhysicsActor)
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Breakable::Release_PhysicsActor()
{
	if (nullptr == m_pPhysicsActor)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pPhysicsActor);

	m_pPhysicsActor = nullptr;
}

HRESULT CLevelDesign_Breakable::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

const _tchar* CLevelDesign_Breakable::Resolve_ModelProtoTag() const
{
	if (m_tBreakableDesc.wstrModelProtoTag.empty())
		return nullptr;

	return m_tBreakableDesc.wstrModelProtoTag.c_str();
}

CLevelDesign_Breakable* CLevelDesign_Breakable::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Breakable* pInstance = new CLevelDesign_Breakable(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Breakable");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Breakable::Clone(void* pArg)
{
	CLevelDesign_Breakable* pInstance = new CLevelDesign_Breakable(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Breakable");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Breakable::Free()
{
	Release_PhysicsActor();

	__super::Free();
}

NS_END