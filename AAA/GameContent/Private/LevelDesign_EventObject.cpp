#include "LevelDesign_EventObject.h"
#include "LevelDesign_Registry.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

namespace
{
	inline constexpr _uint EVENTOBJECT_ANIM_DEFAULT_PASS = 1u;

	struct LD_EVENTOBJECT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		MODEL eModelType;
		_string strAnimNames[LD_ANIM_SLOT_COUNT] = {};
		LD_EVENTOBJECT_RENDER_POLICY eRenderPolicy = { LD_EVENTOBJECT_RENDER_POLICY::DEFAULT };
	};

	static const LD_EVENTOBJECT_CATALOG g_EventObjectCatalog[] =
	{
		{ L"Level1BossDemoBg", CLevelDesign_EventObject::LEVEL1BOSSDEMOBG_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Level1BossDemoBg/Level1BossDemoBg.ysh",
		MODEL::ANIM, { "DemoAppear2", "DemoAppear2AfterWait", "DemoAppear2BeforWait", "" }, LD_EVENTOBJECT_RENDER_POLICY::LEVEL1_BOSS_DEMO_BG },
		{ L"SlopeBoardA", CLevelDesign_EventObject::SLOPEBOARD_A_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardA.ysh",
		MODEL::ANIM, { "LandBack", "LandFront", "LandStartFront", "" }, LD_EVENTOBJECT_RENDER_POLICY::SLOPEBOARD_A },
		{ L"SlopeBoardC", CLevelDesign_EventObject::SLOPEBOARD_C_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardC.ysh",
		MODEL::ANIM, { "FallenWait", "Wait", "", "" }, LD_EVENTOBJECT_RENDER_POLICY::SLOPEBOARD_C },
	};

	static const LD_EVENTOBJECT_CATALOG* Find_EventObjectCatalog(const _wstring& wstrObjName)
	{
		for (const LD_EVENTOBJECT_CATALOG& Entry : g_EventObjectCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, wstrObjName.c_str()))
				return &Entry;
		}

		return nullptr;
	}

#pragma region LEVEL1_BOSS_DEMO_BG
	inline constexpr _uint LEVEL1_BOSS_DEMO_BG_GLASS_PASS = 0u;

	_bool Is_Level1BossDemoBgGlassMesh(const string& strMeshName)
	{
		return "GlassSideM__MlBossGlassC" == strMeshName || "GlassM__MlBossGlassC" == strMeshName;
	}
#pragma endregion
}

NS_BEGIN(Client)

CLevelDesign_EventObject::CLevelDesign_EventObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_EventObject::CLevelDesign_EventObject(const CLevelDesign_EventObject& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tEventObjectDesc(Prototype.m_tEventObjectDesc)
{
}

HRESULT CLevelDesign_EventObject::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_EventObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tEventObjectDesc = *static_cast<const LD_EVENTOBJECT_DESC*>(pArg);

	if (FAILED(Validate_Desc()))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	switch (m_tEventObjectDesc.eRenderPolicy)
	{
	case LD_EVENTOBJECT_RENDER_POLICY::LEVEL1_BOSS_DEMO_BG:
		if (FAILED(Ready_Level1BossDemoBg())) return E_FAIL;
		break;

	case LD_EVENTOBJECT_RENDER_POLICY::SLOPEBOARD_A:
		if (FAILED(Ready_SlopeBoardA())) return E_FAIL;
		break;

	case LD_EVENTOBJECT_RENDER_POLICY::SLOPEBOARD_C:
		if (FAILED(Ready_SlopeBoardC())) return E_FAIL;
		break;

	case LD_EVENTOBJECT_RENDER_POLICY::DEFAULT:
	default:
		break;
	}

	return S_OK;
}

void CLevelDesign_EventObject::Update(_float fTimeDelta)
{
	if (m_pGameInstance_Proxy->Key_Down(DIK_F3)
		&& LD_EVENTOBJECT_RENDER_POLICY::LEVEL1_BOSS_DEMO_BG == m_tEventObjectDesc.eRenderPolicy)
	{
		Play_EventAnimation(0u, false);
	}

	m_pAnimatorCom->Update(fTimeDelta);
}

void CLevelDesign_EventObject::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_EventObject::Render()
{
	switch (m_tEventObjectDesc.eRenderPolicy)
	{
	case LD_EVENTOBJECT_RENDER_POLICY::LEVEL1_BOSS_DEMO_BG:
		return Render_Level1BossDemoBg();

	default:
		return Render_Default();
	}
}

void CLevelDesign_EventObject::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

_bool CLevelDesign_EventObject::Play_EventAnimation(_uint iAnimSlot, _bool bLoop)
{
	if (LD_ANIM_SLOT_COUNT <= iAnimSlot)
		return false;

	return Play_EventAnimation(m_tEventObjectDesc.strAnimNames[iAnimSlot], bLoop);
}

_bool CLevelDesign_EventObject::Play_EventAnimation(const _string& strAnimName, _bool bLoop)
{
	if (nullptr == m_pAnimatorCom || nullptr == m_pModelCom || strAnimName.empty())
		return false;
	if (m_pModelCom->Get_AnimationIndex(strAnimName) < 0)
		return false;

	m_pAnimatorCom->Play(strAnimName, bLoop, true);
	return true;
}

void CLevelDesign_EventObject::Register_LevelDesignSpecs()
{
	for (const LD_EVENTOBJECT_CATALOG& Entry : g_EventObjectCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_Gimmick";
		Spec.eCategory = LD_CATEGORY::GIMMICK;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.eModelType = Entry.eModelType;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.ModelRequirements =
		{
			{ Entry.pModelProtoTag, Entry.pModelPath, Entry.eModelType },
		};

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLevelDesign_EventObject::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;
	if (Spec.eCategory != LD_CATEGORY::GIMMICK || Spec.wstrModelProtoTag.empty())
		return false;

	const LD_EVENTOBJECT_CATALOG* pCatalog = Find_EventObjectCatalog(CommonDesc.strObjectName);
	if (nullptr == pCatalog)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
		Desc.strAnimNames[i] = pCatalog->strAnimNames[i];

	Desc.eRenderPolicy = pCatalog->eRenderPolicy;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLevelDesign_EventObject::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_EventObject::Create(pDevice, pContext);
}

HRESULT CLevelDesign_EventObject::Validate_Desc()
{
	if (m_tEventObjectDesc.eCategory != LD_CATEGORY::GIMMICK)
		return E_FAIL;
	if (m_tEventObjectDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Ready_Components()
{
	if (m_tEventObjectDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	const auto& ShaderDesc = MODEL::ANIM == m_tEventObjectDesc.eModelType ? Shader_AnimMesh_PBR : Shader_NonAnimMesh_PBR;

	m_pShaderCom = Add_Component<CShader>(ShaderDesc.iLevelID, ShaderDesc.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tEventObjectDesc.iModelProtoLevel, m_tEventObjectDesc.wstrModelProtoTag.c_str(),
		TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	if (MODEL::ANIM == m_tEventObjectDesc.eModelType)
	{
		CAnimator::ANIMATOR_DESC AnimDesc{};
		AnimDesc.pModel = m_pModelCom;

		m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
		if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Ready_Level1BossDemoBg()
{
	if (nullptr != m_pAnimatorCom)
		Play_EventAnimation(2u, true);
	
	return S_OK;
}

HRESULT CLevelDesign_EventObject::Ready_SlopeBoardA()
{
	if (nullptr == m_pAnimatorCom)
		return S_OK;

	return Play_EventAnimation(2u, false) ? S_OK : E_FAIL;
}

HRESULT CLevelDesign_EventObject::Ready_SlopeBoardC()
{
	if (nullptr == m_pAnimatorCom)
		return S_OK;

	return Play_EventAnimation(1u, false) ? S_OK : E_FAIL;
}

HRESULT CLevelDesign_EventObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Render_Default()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Render_Mesh(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Render_Level1BossDemoBg()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (Is_Level1BossDemoBgGlassMesh(m_pModelCom->Get_MeshName(i)))
		{
			//if (FAILED(Render_Mesh(i, LEVEL1_BOSS_DEMO_BG_GLASS_PASS)))
			//	return E_FAIL;
		}
		else
		{
			if (FAILED(Render_Mesh(i)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Render_SlopeBoardA()
{
	return E_NOTIMPL;
}

HRESULT CLevelDesign_EventObject::Render_SlopeBoardC()
{
	return E_NOTIMPL;
}

HRESULT CLevelDesign_EventObject::Render_Mesh(_uint iMeshIndex)
{
	return Render_Mesh(iMeshIndex, EVENTOBJECT_ANIM_DEFAULT_PASS);
}

HRESULT CLevelDesign_EventObject::Render_Mesh(_uint iMeshIndex, _uint iAnimPassIndex)
{
	const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(iMeshIndex);

	auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind) -> HRESULT
		{
			const _uint iLayerIndex = MTEX_TYPE::UNKNOWN == eType ? 3u : Layer.idx[ETOUI(eType)];
			const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(iMeshIndex, eType);

			if (0u < iTextureCount)
			{
				const _uint iSafeIndex = (iLayerIndex < iTextureCount) ? iLayerIndex : (iTextureCount - 1u);

				if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pConstantName, iMeshIndex, eType, iSafeIndex)))
					return S_OK;
			}

			return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pConstantName, eDefaultKind);
		};

	if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))             return E_FAIL;
	if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))			return E_FAIL;
	if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))                   return E_FAIL;
	if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))               return E_FAIL;

	if (MODEL::ANIM == m_tEventObjectDesc.eModelType)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", iMeshIndex)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(iAnimPassIndex)))
			return E_FAIL;
	}
	else
	{
		const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
		_uint iFlags = Layer.iFlags;
		_float fDissolve = 0.f;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iEnvInstanceFlags", &iFlags, sizeof(_uint)))) return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float)))) return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::DMN))) return E_FAIL;
	}

	if (FAILED(m_pModelCom->Render(iMeshIndex)))
		return E_FAIL;

	return S_OK;
}

CLevelDesign_EventObject* CLevelDesign_EventObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_EventObject* pInstance = new CLevelDesign_EventObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_EventObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_EventObject::Clone(void* pArg)
{
	CLevelDesign_EventObject* pInstance = new CLevelDesign_EventObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_EventObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_EventObject::Free()
{
	__super::Free();
}

NS_END