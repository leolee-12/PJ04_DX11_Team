#include "Preview_DeformCar_Main.h"

#include "GameContent_AnimEvents.h"
#include "GameInstance_proxy.h"

CPreview_DeformCar_Main::CPreview_DeformCar_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
	, m_eEye{ KIRBY_EYE_STATE::IDLE }
{
}

CPreview_DeformCar_Main::CPreview_DeformCar_Main(const CPreview_DeformCar_Main& Prototype)
	: CGameObject{ Prototype }
	, m_eEye{ Prototype.m_eEye }
{
}

HRESULT CPreview_DeformCar_Main::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;

	return S_OK;
}

HRESULT CPreview_DeformCar_Main::Initialize(void* pArg)
{
	if (nullptr != pArg)
		m_Desc = *static_cast<PREVIEW_DEFORMCAR_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Boost", true, true, 0.2f, 1.f, false);

	return S_OK;
}

void CPreview_DeformCar_Main::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	m_pAnimatorCom->Update(fTimeDelta);
}

void CPreview_DeformCar_Main::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CPreview_DeformCar_Main::Render()
{
	if (m_pModelCom->Get_NumMeshes() < MESH_END)
		return E_FAIL;

	if (FAILED(Bind_CommonResources(m_pPBRShaderCom)))
		return E_FAIL;

	if (FAILED(Bind_CommonResources(m_pKirbyShaderCom)))
		return E_FAIL;

	if (Is_MeshVisible(MESH_CAR) && FAILED(Render_PBRMesh(MESH_CAR)))
		return E_FAIL;

	if (Is_MeshVisible(MESH_KIRBY) && FAILED(Render_KirbyMesh(MESH_KIRBY)))
		return E_FAIL;

	if (Is_MeshVisible(MESH_TIRES) && FAILED(Render_PBRMesh(MESH_TIRES)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPreview_DeformCar_Main::Ready_Components()
{
	m_pKirbyShaderCom = Add_Component<CShader>(
		m_Desc.iProtoLevel,
		m_Desc.szKirbyShaderTag,
		TEXT("Com_Shader_Kirby"));
	if (nullptr == m_pKirbyShaderCom)
		return E_FAIL;

	m_pPBRShaderCom = Add_Component<CShader>(
		m_Desc.iProtoLevel,
		m_Desc.szPBRShaderTag,
		TEXT("Com_Shader_PBR"));
	if (nullptr == m_pPBRShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		m_Desc.iProtoLevel,
		m_Desc.szModelTag,
		TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	m_MeshVisible.assign(m_pModelCom->Get_NumMeshes(), true);

	if (FAILED(Ready_EyeTextures()))
		return E_FAIL;

	CAnimator::ANIMATOR_DESC AnimDesc{};
	AnimDesc.pModel = m_pModelCom;
	AnimDesc.strDataFile = m_Desc.strAnimEvents;

	m_pAnimatorCom = Add_Component<CAnimator>(
		TEXT("Com_Animator"),
		CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
		return E_FAIL;

	m_pAnimatorCom->Set_EventCallback(
		[this](const ANIM_EVENT& Event, ANIM_EVENT_PHASE ePhase)
		{
			if (ANIM_EVENT_PHASE::POINT != ePhase)
				return;

			if (EANIM_EVENT::SetEye == static_cast<EANIM_EVENT>(Event.iEventType))
				Set_Eye(static_cast<KIRBY_EYE_STATE>(Event.iIntParam));
		});

	return S_OK;
}

_bool CPreview_DeformCar_Main::Is_MeshVisible(_uint iMeshIndex) const
{
	if (iMeshIndex >= m_MeshVisible.size())
		return false;

	return m_MeshVisible[iMeshIndex];
}

void CPreview_DeformCar_Main::Set_MeshVisible(_uint iMeshIndex, _bool bVisible)
{
	if (iMeshIndex >= m_MeshVisible.size())
		return;

	m_MeshVisible[iMeshIndex] = bVisible;
}

void CPreview_DeformCar_Main::Set_AllMeshVisible(_bool bVisible)
{
	for (auto&& bMeshVisible : m_MeshVisible)
		bMeshVisible = bVisible;
}

void CPreview_DeformCar_Main::Set_SoloMesh(_uint iMeshIndex)
{
	if (iMeshIndex >= m_MeshVisible.size())
		return;

	for (_uint i = 0; i < m_MeshVisible.size(); ++i)
		m_MeshVisible[i] = (i == iMeshIndex);
}

HRESULT CPreview_DeformCar_Main::Ready_EyeTextures()
{
	m_pEyeTextureCom = Add_Component<CTexture>(
		TEXT("Com_EyeTexture"),
		CTexture::Create(
			m_pDevice,
			m_pContext,
			L"../../Resources/YSE/DeformCar/KirbyEye.%02d.dds",
			ETOUI(KIRBY_EYE_STATE::END)));
	if (nullptr == m_pEyeTextureCom)
		return E_FAIL;

	m_pEyeMaskTextureCom = Add_Component<CTexture>(
		TEXT("Com_EyeMaskTexture"),
		CTexture::Create(
			m_pDevice,
			m_pContext,
			L"../../Resources/YSE/DeformCar/KirbyEyeMask.%02d.dds",
			ETOUI(KIRBY_EYE_STATE::END)));
	if (nullptr == m_pEyeMaskTextureCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CPreview_DeformCar_Main::Bind_CommonResources(CShader* pShader)
{
	if (nullptr == pShader)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix(
		"g_ViewMatrix",
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix(
		"g_ProjMatrix",
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPreview_DeformCar_Main::Render_PBRMesh(_uint iMeshIndex)
{
	if (FAILED(m_pModelCom->Bind_Material(
		m_pPBRShaderCom,
		"g_DiffuseTexture",
		iMeshIndex,
		MTEX_TYPE::DIFFUSE,
		0)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_Material(
		m_pPBRShaderCom,
		"g_NormalTexture",
		iMeshIndex,
		MTEX_TYPE::NORMALS,
		0)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_Material(
		m_pPBRShaderCom,
		"g_MRATexture",
		iMeshIndex,
		MTEX_TYPE::METALNESS,
		0)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_BoneMatrices(
		m_pPBRShaderCom,
		"g_BoneMatrices",
		iMeshIndex)))
		return E_FAIL;

	if (FAILED(m_pPBRShaderCom->Begin(1)))
		return E_FAIL;

	return m_pModelCom->Render(iMeshIndex);
}

HRESULT CPreview_DeformCar_Main::Render_KirbyMesh(_uint iMeshIndex)
{
	if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(
		m_pKirbyShaderCom,
		"g_EyeTexture",
		ETOUI(m_eEye))))
		return E_FAIL;

	if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(
		m_pKirbyShaderCom,
		"g_EyeMaskTexture",
		ETOUI(m_eEye))))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_Material(
		m_pKirbyShaderCom,
		"g_SkinTexture",
		iMeshIndex,
		MTEX_TYPE::UNKNOWN,
		1)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_Material(
		m_pKirbyShaderCom,
		"g_MouthTexture",
		iMeshIndex,
		MTEX_TYPE::UNKNOWN,
		2)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_BoneMatrices(
		m_pKirbyShaderCom,
		"g_BoneMatrices",
		iMeshIndex)))
		return E_FAIL;

	if (FAILED(m_pKirbyShaderCom->Bind_RawValue(
		"g_vBodyColor",
		&m_vBodyColor,
		sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pKirbyShaderCom->Bind_RawValue(
		"g_vFootColor",
		&m_vFootColor,
		sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pKirbyShaderCom->Bind_RawValue(
		"g_vBlushColor",
		&m_vBlushColor,
		sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pKirbyShaderCom->Begin(0)))
		return E_FAIL;

	return m_pModelCom->Render(iMeshIndex);
}

CPreview_DeformCar_Main* CPreview_DeformCar_Main::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPreview_DeformCar_Main* pInstance = new CPreview_DeformCar_Main(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPreview_DeformCar_Main");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPreview_DeformCar_Main::Clone(void* pArg)
{
	CPreview_DeformCar_Main* pInstance = new CPreview_DeformCar_Main(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPreview_DeformCar_Main");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPreview_DeformCar_Main::Free()
{
	__super::Free();
}
