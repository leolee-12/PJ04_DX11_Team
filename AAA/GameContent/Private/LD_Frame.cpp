#include "LD_Frame.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"
#include "World_BlendCollector.h"

#include "GameContent_Events.h"

NS_BEGIN(Client)

CLD_Frame::CLD_Frame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
	, m_bCutReset{ false }
{
}

CLD_Frame::CLD_Frame(const CLD_Frame& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tStaticModelDesc(Prototype.m_tStaticModelDesc)
	, m_bCutReset{ false }
{
}

HRESULT CLD_Frame::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_Frame::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tStaticModelDesc = *static_cast<const LD_STATIC_MODEL_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	Cache_BlendMeshIndices();

	if (FAILED(Ready_CullingState(m_pModelCom)))
		return E_FAIL;

	return Validate_Initialized();
}

HRESULT CLD_Frame::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tStaticModelDesc.strObjectName.c_str()))
		return E_FAIL;

	if (LD_CATEGORY::GIMMICK != m_tStaticModelDesc.eCategory)
		return E_FAIL;

	if (m_tStaticModelDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
		return E_FAIL;

	if (m_tStaticModelDesc.bUseCollMesh)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom || 0u == m_pModelCom->Get_NumMeshes())
		return E_FAIL;

	return S_OK;
}

void CLD_Frame::Update(_float fTimeDelta)
{
	if (m_fFirstWait < START_TIMMER)
	{
		m_fFirstWait += fTimeDelta;
		if (m_fFirstWait >= START_TIMMER)
			m_bStarted = true;
	}
}

void CLD_Frame::Late_Update(_float fTimeDelta)
{
	if (!m_bActive || Is_Dead())
		return;

	if (m_bCutReset)
	{
		m_bCutReset = false;
		m_fCutCursor = 0.f;
		m_iLastCutIndex = 0;
		m_iCreditFired = 0;
		m_bCreditClosed = false;
		m_fEndHold = 0.f;
		m_bEndFadeFired = false;
	}

	if (m_bStarted)
	{
		const _float fCutPeriod = max(max(m_fCutHold, 0.f) + max(m_fCutFade, 0.f), 0.0001f);
		m_fCutCursor = min(m_fCutCursor + fTimeDelta / fCutPeriod, static_cast<_float>(CUT_TEXTURE_COUNT - 1u));

		Tick_CreditSignal();
		Tick_EndFade(fTimeDelta);
	}

	Check_Visible();
	Submit_RenderGroups();
	Submit_BlendMeshes();
}

HRESULT CLD_Frame::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Render_Mesh(i, MESH_LAYER_RENDER_KIND::MAIN)))
			return E_FAIL;
	}

	return S_OK;
}

void CLD_Frame::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

HRESULT CLD_Frame::Render_BlendMesh(_uint iMeshIndex)
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Mesh(iMeshIndex, MESH_LAYER_RENDER_KIND::MAIN_BLEND);
}

HRESULT CLD_Frame::Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer)
{
	if (FAILED(__super::Apply_EditMeshLayer(iModelSlot, iMesh, Layer)))
		return E_FAIL;

	Cache_BlendMeshIndices();
	return S_OK;
}

void CLD_Frame::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::GIMMICK;
	Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
	Spec.eModelType = MODEL::NONANIM;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	LD_MODEL_REQUIREMENT ModelRequirement{ MODEL_PROTO_TAG, MODEL_PATH, MODEL::NONANIM, false };
	ModelRequirement.bUseTextureHubLoader = false;
	Spec.ModelRequirements = { ModelRequirement };

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_Frame::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK
		|| Spec.eModelType != MODEL::NONANIM
		|| Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
	{
		return false;
	}

	LD_STATIC_MODEL_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.bUseCollMesh = false;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_Frame::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_Frame::Create(pDevice, pContext);
}

HRESULT CLD_Frame::Ready_RenderComponents()
{
	if (m_tStaticModelDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	m_pShaderCom = Add_Component<CShader>(Shader_World_NonAnim.iLevelID, Shader_World_NonAnim.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		m_tStaticModelDesc.iModelProtoLevel,
		m_tStaticModelDesc.wstrModelProtoTag,
		TEXT("Com_Model"));

	if (nullptr == m_pModelCom)
		return E_FAIL;

	m_pCutTextureCom = Add_Component<CTexture>(TEXT("Com_CutTexture"), CTexture::Create(m_pDevice, m_pContext, CUT_TEXTURE_PATH, CUT_TEXTURE_COUNT));

	if (nullptr == m_pCutTextureCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_Frame::Bind_ShaderResources()
{
	if (FAILED(MeshLayerBinder::Bind_WorldViewProj(m_pShaderCom, m_pTransformCom, m_pGameInstance_Proxy, m_eProjType)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_Frame::Render_Mesh(_uint iMeshIndex, MESH_LAYER_RENDER_KIND eKind)
{
	const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(iMeshIndex);
	const _bool bUseUnknownPass =
		0u == m_pModelCom->Get_MeshTextureCount(iMeshIndex, MTEX_TYPE::DIFFUSE)
		&& 0u < m_pModelCom->Get_MeshTextureCount(iMeshIndex, MTEX_TYPE::UNKNOWN);

	MESH_LAYER_BIND_CONTEXT Ctx{};
	Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
	Ctx.iMesh = iMeshIndex;
	Ctx.pLayer = &Layer;
	Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
	Ctx.eKind = eKind;
	Ctx.iFallbackPass = bUseUnknownPass ? ETOUI(WORLD_PASS::UKWN) : ETOUI(WORLD_PASS::DMN);

	_uint iPass = 0u;
	const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
	if (FAILED(hrBind))
		return E_FAIL;
	if (S_FALSE == hrBind)
		return S_OK;

	if (ETOI(WORLD_PASS::CUT_CROSSFADE) == Layer.iPass && FAILED(Bind_CutTextures()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(iPass)))
		return E_FAIL;

	return m_pModelCom->Render(iMeshIndex);
}

void CLD_Frame::Cache_BlendMeshIndices()
{
	m_BlendMeshIndices.clear();

	if (nullptr == m_pModelCom)
		return;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (Is_WorldBlendPass(m_pModelCom->Get_MeshLayer(i).iPass))
			m_BlendMeshIndices.push_back(i);
	}
}

void CLD_Frame::Submit_BlendMeshes()
{
	if (!m_bVisible || m_BlendMeshIndices.empty() || nullptr == m_pModelCom)
		return;

	if (nullptr == m_pBlendCollector)
		m_pBlendCollector = CWorld_BlendCollector::Find(m_pGameInstance_Proxy);

	if (nullptr == m_pBlendCollector)
		return;

	const _float4x4* pWorld = m_pTransformCom->Get_WorldMatrixPtr();

	for (_uint iMeshIndex : m_BlendMeshIndices)
		m_pBlendCollector->Submit(this, this, m_pModelCom, pWorld, iMeshIndex);
}

void CLD_Frame::Tick_CreditSignal()
{
	const _float fPeriod = max(max(m_fCutHold, 0.f) + max(m_fCutFade, 0.f), 0.0001f);
	const _float fFadeRatio = max(m_fCutFade, 0.f) / fPeriod;

	const _int iCut = static_cast<_int>(m_fCutCursor + fFadeRatio);
	if (iCut == m_iLastCutIndex)
		return;

	m_iLastCutIndex = iCut;

	if (m_iCreditFired < CREDIT_COUNT)
	{
		const _int iTargetCut = CREDIT_FIRST_CUT + CREDIT_CUT_STRIDE * m_iCreditFired;
		if (iCut >= iTargetCut)
		{
			++m_iCreditFired;
			m_pGameInstance_Proxy->Publish(EventTag::Credits_Next, nullptr);
			return;
		}
	}

	if (!m_bCreditClosed && CREDIT_CLOSE_CUT >= 0 &&
		m_iCreditFired >= CREDIT_COUNT && iCut >= CREDIT_CLOSE_CUT)
	{
		m_bCreditClosed = true;
		m_pGameInstance_Proxy->Publish(EventTag::Credits_Next, nullptr);
	}
}

void CLD_Frame::Tick_EndFade(_float fTimeDelta)
{
	if (m_bEndFadeFired || !m_bCreditClosed)
		return;

	if (m_fCutCursor < static_cast<_float>(CUT_TEXTURE_COUNT - 1u))
		return;

	m_fEndHold += fTimeDelta;

	if (m_fEndHold >= max(m_fCutHold, 0.f))
	{
		m_bEndFadeFired = true;
		m_pGameInstance_Proxy->Publish(TEXT("Ending.BlackFade"), nullptr);
	}
}

HRESULT CLD_Frame::Bind_CutTextures()
{
	if (nullptr == m_pCutTextureCom)
		return E_FAIL;

	const _uint iSrc = min(static_cast<_uint>(m_fCutCursor), CUT_TEXTURE_COUNT - 1u);
	const _uint iDst = min(iSrc + 1u, CUT_TEXTURE_COUNT - 1u);

	const _float fHold = max(m_fCutHold, 0.f);
	const _float fFade = max(m_fCutFade, 0.f);
	const _float fElapsed = (m_fCutCursor - static_cast<_float>(iSrc)) * max(fHold + fFade, 0.0001f);
	const _float fBlend = min(max((fElapsed - fHold) / max(fFade, 0.0001f), 0.f), 1.f);

	if (FAILED(m_pCutTextureCom->Bind_ShaderResource(m_pShaderCom, "g_UnknownTexture", iSrc)))
		return E_FAIL;

	if (FAILED(m_pCutTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ExtraRTexture", iDst)))
		return E_FAIL;

	return m_pShaderCom->Bind_RawValue("g_fCutBlend", &fBlend, sizeof(_float));
}

CLD_Frame* CLD_Frame::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_Frame* pInstance = new CLD_Frame(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_Frame");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_Frame::Clone(void* pArg)
{
	CLD_Frame* pInstance = new CLD_Frame(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_Frame");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_Frame::Free()
{
	m_pCutTextureCom = nullptr;
	m_pBlendCollector = nullptr;
	m_BlendMeshIndices.clear();

	__super::Free();
}

NS_END