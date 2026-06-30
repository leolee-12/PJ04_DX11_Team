#include "LevelDesign_Starblock.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"


namespace
{
	struct LD_STARBLOCK_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		const _float fRadius;
	};

	static const LD_STARBLOCK_CATALOG g_BreakableCatalog[] =
	{
		{ L"StarBlock", CLevelDesign_Starblock::STARBLOCK_H1W1_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Star/H1W1.ysh", 0.25f },
		{ L"StarBlockBig", CLevelDesign_Starblock::STARBLOCK_H3W3_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Star/H3W3.ysh" , 1.f },
	};

	static const LD_STARBLOCK_CATALOG* Find_BreakableCatalog(const _wstring& wstrObjName)
	{
		for (const LD_STARBLOCK_CATALOG& Entry : g_BreakableCatalog)
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
		pOutDesc->wstrModelProtoTag = CLevelDesign_Starblock::STARBLOCK_H1W1_MODEL_PROTO_TAG;

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

CLevelDesign_Starblock::CLevelDesign_Starblock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLDInhalable(pDevice, pContext)
{
}

CLevelDesign_Starblock::CLevelDesign_Starblock(const CLevelDesign_Starblock& Prototype)
	: CLDInhalable(Prototype)
	, m_tBreakableDesc(Prototype.m_tBreakableDesc)
{
}

HRESULT CLevelDesign_Starblock::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Starblock::Initialize(void* pArg)
{
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

	auto* pCatalog = Find_BreakableCatalog(m_tBreakableDesc.strObjectName);

	m_fColliderRadius = pCatalog->fRadius;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_PhysicsActor_Box()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Starblock::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);

	if (nullptr != m_pModelCom)
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Starblock::Render()
{
	if (!m_bActive)
		return S_OK;

	if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

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

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Starblock::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

#pragma region Damageable
void CLevelDesign_Starblock::Damaged(const ATTACK_INFO& tInfo)
{
	UNREFERENCED_PARAMETER(tInfo);
	
	Set_Active(false);
}
#pragma endregion

#pragma region Inhalable
void CLevelDesign_Starblock::Be_Captured(CGameObject* pInhaler)
{
	Release_PhysicsActor();

	__super::Be_Captured(pInhaler);
}
#pragma endregion

HRESULT CLevelDesign_Starblock::Validate_Desc()
{
	if (m_tBreakableDesc.eCategory != LD_CATEGORY::BREAKABLE)
		return E_FAIL;
	if (m_tBreakableDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Starblock::Ready_Components()
{
	const _tchar* pModelProtoTag = Resolve_ModelProtoTag();
	if (nullptr == pModelProtoTag)
		return E_FAIL;

	const auto& ShaderDesc = Shader_NonAnimMesh_PBR;

	m_pShaderCom = Add_Component<CShader>(ShaderDesc.iLevelID, ShaderDesc.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tBreakableDesc.iModelProtoLevel, pModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Starblock::Ready_PhysicsActor_Box()
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

void CLevelDesign_Starblock::Release_PhysicsActor()
{
	if (nullptr == m_pPhysicsActor)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pPhysicsActor);

	m_pPhysicsActor = nullptr;
}

HRESULT CLevelDesign_Starblock::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

const _tchar* CLevelDesign_Starblock::Resolve_ModelProtoTag() const
{
	if (m_tBreakableDesc.wstrModelProtoTag.empty())
		return nullptr;

	return m_tBreakableDesc.wstrModelProtoTag.c_str();
}

void CLevelDesign_Starblock::Register_LevelDesignSpecs()
{
	for (const LD_STARBLOCK_CATALOG& Entry : g_BreakableCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_Gimmick";
		Spec.eCategory = LD_CATEGORY::BREAKABLE;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.ModelRequirements =
		{
			{ Entry.pModelProtoTag, Entry.pModelPath }
		};

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLevelDesign_Starblock::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
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

CGameObject* CLevelDesign_Starblock::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_Starblock::Create(pDevice, pContext);
}

CLevelDesign_Starblock* CLevelDesign_Starblock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Starblock* pInstance = new CLevelDesign_Starblock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Starblock");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Starblock::Clone(void* pArg)
{
	CLevelDesign_Starblock* pInstance = new CLevelDesign_Starblock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Starblock");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Starblock::Free()
{
	Release_PhysicsActor();

	__super::Free();
}

NS_END