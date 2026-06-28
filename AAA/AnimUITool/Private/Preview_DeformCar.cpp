#include "Preview_DeformCar.h"

#include "Animator.h"
#include "GameContent_AnimEvents.h"
#include "GameInstance_proxy.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"

CPreview_DeformCar::CPreview_DeformCar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
	, m_eEye{ KIRBY_EYE_STATE::IDLE }
{
}

CPreview_DeformCar::CPreview_DeformCar(const CPreview_DeformCar& Prototype)
	: CGameObject{ Prototype }
	, m_eEye{ Prototype.m_eEye }
{
}

HRESULT CPreview_DeformCar::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;

	return S_OK;
}

HRESULT CPreview_DeformCar::Initialize(void* pArg)
{
	if (nullptr != pArg)
		m_Desc = *static_cast<PREVIEW_DEFORMCAR_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true, 0.2f, 1.f, false);

	return S_OK;
}

void CPreview_DeformCar::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	m_pAnimatorCom->Update(fTimeDelta);
}

void CPreview_DeformCar::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CPreview_DeformCar::Render()
{
	if (m_pModelCom->Get_NumMeshes() < MESH_END)
		return E_FAIL;

	if (FAILED(Bind_CommonResources(m_pPBRShaderCom)))
		return E_FAIL;

	if (FAILED(Bind_CommonResources(m_pKirbyShaderCom)))
		return E_FAIL;

	if (FAILED(Render_PBRMesh(MESH_CAR)))
		return E_FAIL;

	if (FAILED(Render_KirbyMesh(MESH_KIRBY)))
		return E_FAIL;

	if (FAILED(Render_PBRMesh(MESH_TIRES)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPreview_DeformCar::Ready_Components()
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

HRESULT CPreview_DeformCar::Ready_EyeTextures()
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

HRESULT CPreview_DeformCar::Bind_CommonResources(CShader* pShader)
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

HRESULT CPreview_DeformCar::Render_PBRMesh(_uint iMeshIndex)
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

HRESULT CPreview_DeformCar::Render_KirbyMesh(_uint iMeshIndex)
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

CPreview_DeformCar* CPreview_DeformCar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPreview_DeformCar* pInstance = new CPreview_DeformCar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPreview_DeformCar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPreview_DeformCar::Clone(void* pArg)
{
	CPreview_DeformCar* pInstance = new CPreview_DeformCar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPreview_DeformCar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPreview_DeformCar::Free()
{
	__super::Free();
}
