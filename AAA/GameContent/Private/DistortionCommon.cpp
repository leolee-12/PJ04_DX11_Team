#include "DistortionCommon.h"

#include "GameContent_const.h"
#include "Effect_MeshCommon.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"

CDistortionCommon::CDistortionCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_NonParticle{ pDevice, pContext }
	, m_bBillboard{ false }
	, m_bRadialFromUV{ true }
	, m_fDistortionStrength{ 0.01f }
{
}

CDistortionCommon::CDistortionCommon(const CDistortionCommon& Prototype)
	: CEffect_NonParticle{ Prototype }
	, m_bBillboard{ Prototype.m_bBillboard }
	, m_bRadialFromUV{ Prototype.m_bRadialFromUV }
	, m_fDistortionStrength{ Prototype.m_fDistortionStrength }
	, m_iModelLevel{ Prototype.m_iModelLevel }
	, m_wstrModelTag{ Prototype.m_wstrModelTag }
{
}

HRESULT CDistortionCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CDistortionCommon::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	DISTORTION_COMMON_DESC DistortionDesc =
		*static_cast<DISTORTION_COMMON_DESC*>(pArg);

	if (DistortionDesc.wstrModelTag.empty() ||
		DistortionDesc.bUseTextureCom == false ||
		DistortionDesc.wstrTextureTag.empty())
		return E_FAIL;

	m_iModelLevel = DistortionDesc.iModelLevel;
	m_wstrModelTag = DistortionDesc.wstrModelTag;
	m_bCustomShader = true;

	if (FAILED(__super::Initialize(&DistortionDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CDistortionCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_bActive == false ||
		m_bUseTextureCom == false ||
		nullptr == m_pTextureCom)
		return;

	Compute_CombinedWorldMatrix();
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::DISTORTION, this);
}

HRESULT CDistortionCommon::Render()
{
	_float fAlpha{};
	if (FAILED(Bind_DistortionResources(&fAlpha)))
		return E_FAIL;

	if (fabsf(m_fDistortionStrength) <= Helper::fEpsilon ||
		fAlpha <= Helper::fEpsilon)
		return S_OK;

	const _uint iPass = m_iDepthIgnore == DepthMode::DEPTH_DEFAULT ? 0u : 1u;
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CDistortionCommon::Ready_Components()
{
	m_pShaderCom = Add_Component<CShader>(
		Shader_Distortion.iLevelID,
		Shader_Distortion.szProtoTag,
		TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		m_iModelLevel,
		m_wstrModelTag,
		TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CDistortionCommon::Bind_DistortionResources(_float* pOutAlpha)
{
	if (nullptr == pOutAlpha ||
		nullptr == m_pShaderCom ||
		nullptr == m_pTextureCom ||
		nullptr == m_pModelCom)
		return E_FAIL;

	_float4x4 WorldMatrix = m_CombinedWorldMatrix;
	if (m_bBillboard == true)
	{
		WorldMatrix = Make_BillboardWorldMatrix(m_CombinedWorldMatrix);
		EffectMesh::Apply_BillboardRoll(WorldMatrix, m_fRoll);
	}

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
		return E_FAIL;
	if (FAILED(Bind_ViewProjectionMatrices()))
		return E_FAIL;
	_bool bViewSpace = !m_bBillboard;
	m_pShaderCom->Bind_RawValue("g_bViewSpaceNormal", &bViewSpace, sizeof(_bool));
	m_pShaderCom->Bind_RawValue("g_bFlipGreen", &m_bFlipGreen, sizeof(_bool));
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	_float fAlpha = m_fAlpha;
	Helper::FloatClamp(fAlpha, 0.f, 1.f);
	fAlpha *= Get_FadeOutAlpha();
	*pOutAlpha = fAlpha;

	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_bRadialFromUV", &m_bRadialFromUV, sizeof(m_bRadialFromUV))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_fStrength", &m_fDistortionStrength, sizeof(m_fDistortionStrength))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_fAlpha", &fAlpha, sizeof(fAlpha))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_vTiling", &m_vTextureTiling, sizeof(m_vTextureTiling))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_vOffset", &m_vCurTextureUVOffset, sizeof(m_vCurTextureUVOffset))))
		return E_FAIL;

	return S_OK;
}

CDistortionCommon* CDistortionCommon::Create(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CDistortionCommon* pInstance = new CDistortionCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CDistortionCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDistortionCommon::Clone(void* pArg)
{
	CDistortionCommon* pInstance = new CDistortionCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CDistortionCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDistortionCommon::Free()
{
	__super::Free();
}
