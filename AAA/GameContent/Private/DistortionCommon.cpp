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
	, m_bFlipGreen{ false }
	, m_bUseUVEdgeFade{ false }
	, m_iUVEdgeFadeAxis{ 0 }
	, m_fUVEdgeFadeStartRange{ 0.1f }
	, m_fUVEdgeFadeEndRange{ 0.1f }
	, m_fUVEdgeFadePower{ 1.f }
	, m_bLinearReveal{ false }
	, m_iLinearRevealAxis{ 0 }
	, m_bLinearRevealReverse{ false }
	, m_fLinearRevealStartRatio{ 0.f }
	, m_fLinearRevealEndRatio{ 1.f }
	, m_bLinearHide{ false }
	, m_iLinearHideAxis{ 0 }
	, m_bLinearHideReverse{ false }
	, m_fLinearHideStartRatio{ 0.f }
	, m_fLinearHideEndRatio{ 1.f }
	, m_fLinearRevealRatio{ 1.f }
	, m_fLinearHideRatio{ 1.f }
{
}

CDistortionCommon::CDistortionCommon(const CDistortionCommon& Prototype)
	: CEffect_NonParticle{ Prototype }
	, m_bBillboard{ Prototype.m_bBillboard }
	, m_bRadialFromUV{ Prototype.m_bRadialFromUV }
	, m_fDistortionStrength{ Prototype.m_fDistortionStrength }
	, m_bFlipGreen{ Prototype.m_bFlipGreen }
	, m_bUseUVEdgeFade{ Prototype.m_bUseUVEdgeFade }
	, m_iUVEdgeFadeAxis{ Prototype.m_iUVEdgeFadeAxis }
	, m_fUVEdgeFadeStartRange{ Prototype.m_fUVEdgeFadeStartRange }
	, m_fUVEdgeFadeEndRange{ Prototype.m_fUVEdgeFadeEndRange }
	, m_fUVEdgeFadePower{ Prototype.m_fUVEdgeFadePower }
	, m_bLinearReveal{ Prototype.m_bLinearReveal }
	, m_iLinearRevealAxis{ Prototype.m_iLinearRevealAxis }
	, m_bLinearRevealReverse{ Prototype.m_bLinearRevealReverse }
	, m_fLinearRevealStartRatio{ Prototype.m_fLinearRevealStartRatio }
	, m_fLinearRevealEndRatio{ Prototype.m_fLinearRevealEndRatio }
	, m_bLinearHide{ Prototype.m_bLinearHide }
	, m_iLinearHideAxis{ Prototype.m_iLinearHideAxis }
	, m_bLinearHideReverse{ Prototype.m_bLinearHideReverse }
	, m_fLinearHideStartRatio{ Prototype.m_fLinearHideStartRatio }
	, m_fLinearHideEndRatio{ Prototype.m_fLinearHideEndRatio }
	, m_iModelLevel{ Prototype.m_iModelLevel }
	, m_wstrModelTag{ Prototype.m_wstrModelTag }
	, m_fLinearRevealRatio{ Prototype.m_fLinearRevealRatio }
	, m_fLinearHideRatio{ Prototype.m_fLinearHideRatio }
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

void CDistortionCommon::Update_Core(
	const _float fTimeDelta,
	const _float fRatio)
{
	__super::Update_Core(fTimeDelta, fRatio);

	if (m_bLinearReveal == false)
		m_fLinearRevealRatio = 1.f;
	else
	{
		const _float fRevealRange =
			m_fLinearRevealEndRatio - m_fLinearRevealStartRatio;
		if (fabsf(fRevealRange) <= Helper::fEpsilon)
			m_fLinearRevealRatio =
				fRatio >= m_fLinearRevealEndRatio ? 1.f : 0.f;
		else
		{
			m_fLinearRevealRatio =
				(fRatio - m_fLinearRevealStartRatio) / fRevealRange;
			Helper::FloatClamp(m_fLinearRevealRatio, 0.f, 1.f);
		}
	}

	if (m_bLinearHide == false)
		m_fLinearHideRatio = 1.f;
	else
	{
		const _float fHideRange =
			m_fLinearHideEndRatio - m_fLinearHideStartRatio;
		if (fabsf(fHideRange) <= Helper::fEpsilon)
			m_fLinearHideRatio =
				fRatio >= m_fLinearHideEndRatio ? 0.f : 1.f;
		else
		{
			_float fHideProgress =
				(fRatio - m_fLinearHideStartRatio) / fHideRange;
			Helper::FloatClamp(fHideProgress, 0.f, 1.f);
			m_fLinearHideRatio = 1.f - fHideProgress;
		}
	}
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
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_bUseUVEdgeFade", &m_bUseUVEdgeFade, sizeof(m_bUseUVEdgeFade))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_iUVEdgeFadeAxis", &m_iUVEdgeFadeAxis, sizeof(m_iUVEdgeFadeAxis))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_fUVEdgeFadeStartRange", &m_fUVEdgeFadeStartRange, sizeof(m_fUVEdgeFadeStartRange))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_fUVEdgeFadeEndRange", &m_fUVEdgeFadeEndRange, sizeof(m_fUVEdgeFadeEndRange))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_fUVEdgeFadePower", &m_fUVEdgeFadePower, sizeof(m_fUVEdgeFadePower))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_bLinearReveal", &m_bLinearReveal, sizeof(m_bLinearReveal))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_fLinearRevealRatio", &m_fLinearRevealRatio, sizeof(m_fLinearRevealRatio))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_iLinearRevealAxis", &m_iLinearRevealAxis, sizeof(m_iLinearRevealAxis))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_bLinearRevealReverse", &m_bLinearRevealReverse, sizeof(m_bLinearRevealReverse))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_bLinearHide", &m_bLinearHide, sizeof(m_bLinearHide))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_fLinearHideRatio", &m_fLinearHideRatio, sizeof(m_fLinearHideRatio))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_iLinearHideAxis", &m_iLinearHideAxis, sizeof(m_iLinearHideAxis))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue(
		"g_bLinearHideReverse", &m_bLinearHideReverse, sizeof(m_bLinearHideReverse))))
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
