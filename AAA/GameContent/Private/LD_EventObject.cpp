#include "LD_EventObject.h"
#include "GameContent_const.h"
#include "Shader_PassMeta.h"

#include "GameInstance.h"

namespace
{
	inline constexpr _uint EVENTOBJECT_ANIM_DEFAULT_PASS = 1u;

	void Log_EventObjectPhysicsWarning(const string& strMessage)
	{
#ifdef _DEBUG
		OutputDebugStringA((strMessage + "\n").c_str());
#else
		UNREFERENCED_PARAMETER(strMessage);
#endif
	}
}

NS_BEGIN(Client)

CLD_EventObject::CLD_EventObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_EventObject::CLD_EventObject(const CLD_EventObject& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tEventObjectDesc(Prototype.m_tEventObjectDesc)
	, m_AnimPlayDescs(Prototype.m_AnimPlayDescs)
{
}

HRESULT CLD_EventObject::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_EventObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tEventObjectDesc = *static_cast<const LD_EVENTOBJECT_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_EventObject::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tEventObjectDesc.eCategory != LD_CATEGORY::GIMMICK)
		return E_FAIL;

	if (m_tEventObjectDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	if (MODEL::ANIM != m_tEventObjectDesc.eModelType && MODEL::NONANIM != m_tEventObjectDesc.eModelType)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom)
		return E_FAIL;

	if (MODEL::ANIM == m_tEventObjectDesc.eModelType)
	{
		if (nullptr == m_pAnimatorCom)
			return E_FAIL;

		for (const auto& AnimDesc : m_AnimPlayDescs)
		{
			if (AnimDesc.strAnimName.empty())
				return E_FAIL;

			if (m_pModelCom->Get_AnimationIndex(AnimDesc.strAnimName) < 0)
				return E_FAIL;
		}
	}

	if (m_MeshVisible.size() != static_cast<size_t>(m_pModelCom->Get_NumMeshes()))
		return E_FAIL;

	if (m_tEventObjectDesc.bUseCollMesh && nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLD_EventObject::Update(_float fTimeDelta)
{
	if (!m_bAnimationActive)
		return;

	m_pAnimatorCom->Update(fTimeDelta);

	if (m_pAnimatorCom->Is_Finished())
		m_bAnimationActive = false;
}

void CLD_EventObject::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bActive || Is_Dead())
		return;

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLD_EventObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (!Should_RenderMesh(i))
			continue;

		if (FAILED(Render_Mesh(i, Resolve_RenderPass(i))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLD_EventObject::Ready_Components()
{
	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_AnimEvents()))
		return E_FAIL;

	m_MeshVisible.assign(static_cast<size_t>(m_pModelCom->Get_NumMeshes()), true);

	if (FAILED(Ready_RigidStatic()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_EventObject::Ready_RenderComponents()
{
	if (m_tEventObjectDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	const auto& ShaderDesc = MODEL::ANIM == m_tEventObjectDesc.eModelType ? Shader_AnimMesh_PBR : Shader_NonAnimMesh_PBR;

	m_pShaderCom = Add_Component<CShader>(ShaderDesc.iLevelID, ShaderDesc.szProtoTag,
		TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		m_tEventObjectDesc.iModelProtoLevel,
		m_tEventObjectDesc.wstrModelProtoTag.c_str(),
		TEXT("Com_Model"));

	if (nullptr == m_pModelCom)
		return E_FAIL;

	if (MODEL::ANIM == m_tEventObjectDesc.eModelType)
	{
		CAnimator::ANIMATOR_DESC AnimDesc{};
		AnimDesc.pModel = m_pModelCom;
		AnimDesc.strDataFile = m_tEventObjectDesc.strAnimEventFile;

		m_pAnimatorCom = Add_Component<CAnimator>(
			TEXT("Com_Animator"),
			CAnimator::Create(m_pDevice, m_pContext));

		if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLD_EventObject::Ready_AnimPlayDescs(const LD_ANIM_PLAY_DESC* pAnimDescs, _uint iCount)
{
	m_AnimPlayDescs.clear();

	if (0u < iCount && nullptr == pAnimDescs)
		return E_FAIL;

	for (_uint i = 0; i < iCount; ++i)
	{
		const LD_ANIM_PLAY_DESC& AnimDesc = pAnimDescs[i];

		if (AnimDesc.strAnimName.empty())
			return E_FAIL;

		if (m_pModelCom->Get_AnimationIndex(AnimDesc.strAnimName) < 0)
			return E_FAIL;

		m_AnimPlayDescs.push_back(AnimDesc);
	}

	return S_OK;
}

const CLD_EventObject::LD_ANIM_PLAY_DESC* CLD_EventObject::Find_AnimPlayDesc(const _string& strAnimName) const
{
	for (const auto& AnimDesc : m_AnimPlayDescs)
	{
		if (AnimDesc.strAnimName == strAnimName)
			return &AnimDesc;
	}

	return nullptr;
}

HRESULT CLD_EventObject::Set_AnimPose(const _string& strAnimName, _float fProgress)
{
	const LD_ANIM_PLAY_DESC* pAnimDesc = Find_AnimPlayDesc(strAnimName);
	if (nullptr == pAnimDesc)
		return E_FAIL;

	m_pAnimatorCom->Play(pAnimDesc->strAnimName, pAnimDesc->bLoop, true, 0.f, pAnimDesc->fSpeed);
	m_pAnimatorCom->Seek(fProgress);
	m_bAnimationActive = false;

	return S_OK;
}

void CLD_EventObject::Play_Anim(const _string& strAnimName)
{
	const LD_ANIM_PLAY_DESC* pAnimDesc = Find_AnimPlayDesc(strAnimName);
	if (nullptr == pAnimDesc)
		return;

	m_pAnimatorCom->Resume();
	m_pAnimatorCom->Play(pAnimDesc->strAnimName, pAnimDesc->bLoop, true, 0.f, pAnimDesc->fSpeed);
	m_bAnimationActive = true;
}

HRESULT CLD_EventObject::Ready_AnimEvents()
{
	if (nullptr == m_pAnimatorCom)
		return S_OK;

	m_pAnimatorCom->Set_EventCallback(
		[this](const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase)
		{
			On_AnimEvent(AnimEvent, ePhase);
		});

	return S_OK;
}

HRESULT CLD_EventObject::Ready_RigidStatic()
{
	Release_RigidStatic();

	if (!m_tEventObjectDesc.bUseCollMesh)
		return S_OK;

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pModelCom)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] RigidStatic skipped: required component is null.");
		return E_FAIL;
	}

	auto pCollisionMesh = m_pModelCom->Get_CollisionMesh();
	if (nullptr == pCollisionMesh)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] RigidStatic skipped: cooked collision mesh is null.");
		return E_FAIL;
	}

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(
		pCollisionMesh,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	if (nullptr == m_pRigidStatic)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] RigidStatic failed: Create_StaticActor returned null.");
		return E_FAIL;
	}

	return S_OK;
}

void CLD_EventObject::Release_RigidStatic()
{
	if (nullptr == m_pRigidStatic)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);

	m_pRigidStatic = nullptr;
}

_int CLD_EventObject::Find_MeshIndex_ByName(const _string& strMeshName) const
{
	if (nullptr == m_pModelCom)
		return -1;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (m_pModelCom->Get_MeshName(i) == strMeshName)
			return static_cast<_int>(i);
	}

	return -1;
}

void CLD_EventObject::On_AnimEvent(const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase)
{
	UNREFERENCED_PARAMETER(AnimEvent);
	UNREFERENCED_PARAMETER(ePhase);
}

void CLD_EventObject::Set_AllMeshesVisible(_bool bVisible)
{
	for (_uint i = 0; i < static_cast<_uint>(m_MeshVisible.size()); ++i)
		m_MeshVisible[i] = bVisible;
}

void CLD_EventObject::Set_MeshVisible(_uint iMeshIndex, _bool bVisible)
{
	if (iMeshIndex >= m_MeshVisible.size())
		return;

	m_MeshVisible[iMeshIndex] = bVisible;
}

_bool CLD_EventObject::Should_RenderMesh(_uint iMeshIndex) const
{
	return m_MeshVisible[iMeshIndex];
}

_uint CLD_EventObject::Resolve_RenderPass(_uint iMeshIndex) const
{
	UNREFERENCED_PARAMETER(iMeshIndex);

	return EVENTOBJECT_ANIM_DEFAULT_PASS;
}

HRESULT CLD_EventObject::Bind_BoneMatrices(_uint iMeshIndex)
{
	return m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", iMeshIndex);
}

HRESULT CLD_EventObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix(
		"g_ViewMatrix",
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
	{
		return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Bind_Matrix(
		"g_ProjMatrix",
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
	{
		return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_EventObject::Render_Mesh(_uint iMeshIndex)
{
	return Render_Mesh(iMeshIndex, EVENTOBJECT_ANIM_DEFAULT_PASS);
}

HRESULT CLD_EventObject::Render_Mesh(_uint iMeshIndex, _uint iAnimPassIndex)
{
	const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(iMeshIndex);

	auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind)
		-> HRESULT
		{
			const _uint iLayerIndex = MTEX_TYPE::UNKNOWN == eType ? 3u : Layer.idx[ETOUI(eType)];
			const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(iMeshIndex, eType);

			if (0u < iTextureCount)
			{
				const _uint iSafeIndex = iLayerIndex < iTextureCount ? iLayerIndex : iTextureCount - 1u;

				if (SUCCEEDED(m_pModelCom->Bind_Material(
					m_pShaderCom,
					pConstantName,
					iMeshIndex,
					eType,
					iSafeIndex)))
				{
					return S_OK;
				}
			}

			return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(
				m_pShaderCom,
				pConstantName,
				eDefaultKind);
		};

	if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))
		return E_FAIL;

	if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))
		return E_FAIL;

	if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))
		return E_FAIL;

	if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))
		return E_FAIL;

	if (MODEL::ANIM == m_tEventObjectDesc.eModelType)
	{
		if (FAILED(Bind_BoneMatrices(iMeshIndex)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(iAnimPassIndex)))
			return E_FAIL;
	}
	else
	{
		const _uint iUVIndex = Layer.iUVIndex <= 3u ? Layer.iUVIndex : 0u;
		_uint iFlags = Layer.iFlags;
		_float fDissolve = 0.f;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iEnvInstanceFlags", &iFlags, sizeof(_uint))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::DMN)))
			return E_FAIL;
	}

	if (FAILED(m_pModelCom->Render(iMeshIndex)))
		return E_FAIL;

	return S_OK;
}

void CLD_EventObject::Free()
{
	Release_RigidStatic();

	__super::Free();
}

NS_END