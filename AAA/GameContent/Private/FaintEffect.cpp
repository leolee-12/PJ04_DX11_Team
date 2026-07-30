#include "FaintEffect.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "MeshCommon.h"
#include "MeshEmitterCommon.h"

CFaintEffect::CFaintEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CFaintEffect::CFaintEffect(const CFaintEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CFaintEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFaintEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CFaintEffect::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CFaintEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CFaintEffect::Late_Update(_float fTimeDelta)
{
	Update_ContainerTilt(fTimeDelta);

	__super::Late_Update(fTimeDelta);
}

HRESULT CFaintEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CFaintEffect::Ready_EffectPartObjects()
{
	// Both Boss models expose UNKNOWN slots only (Sample.png dummy).
	CMeshCommon::MESH_COMMON_DESC tMesh{};
	tMesh.iModelLevel        = m_iPrototypeLevel;
	tMesh.bUseDiffuseTexture = false;
	tMesh.bUseNormalTexture  = false;
	tMesh.bUseMRATexture     = false;
	tMesh.bUseUnknownTexture = true;
	tMesh.bUseTextureCom     = false;
	tMesh.bUseMaskCom        = false;
	tMesh.bCustomShader      = false;
	tMesh.iShaderLevel       = 0;
	tMesh.wstrShaderTag      = L"";
	tMesh.wstrModelTag       = STAR_MODEL_TAG;

	// StarSmooth Model Part 00 ~ 02
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Star_00", &tMesh)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Star_01", &tMesh)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Star_02", &tMesh)))
		return E_FAIL;

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel        = m_iPrototypeLevel;
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseNormalTexture  = false;
	tDesc.bUseMRATexture     = false;
	tDesc.bUseUnknownTexture = true;
	tDesc.bUseTextureCom     = false;
	tDesc.bUseMaskCom        = false;
	tDesc.bCustomShader      = false;
	tDesc.iShaderLevel       = 0;
	tDesc.wstrShaderTag      = L"";
	tDesc.wstrModelTag       = SMOKE_MODEL_TAG;

	// SmokeLowPoly Model Part 00 ~ 02
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Smoke_00", &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Smoke_01", &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Smoke_02", &tDesc)))
		return E_FAIL;

	return S_OK;
}

void CFaintEffect::Update_ContainerTilt(_float fTimeDelta)
{
	if (m_bIsPlay == false)
		return;

	constexpr _float fPrecessPeriod = 360.f / TILT_PRECESS_DEG_SEC;
	m_fTiltAccTime = fmodf(m_fTiltAccTime + fTimeDelta, fPrecessPeriod);

	const _float fWobble = sinf(
		m_fTiltAccTime * XM_2PI * TILT_WOBBLE_CYCLES / fPrecessPeriod);

	const _float fTilt = TILT_ANGLE_DEG *
		(1.f - TILT_WOBBLE_DEPTH + TILT_WOBBLE_DEPTH * fWobble);

	const _matrix matTilt =
		XMMatrixRotationZ(XMConvertToRadians(fTilt)) *
		XMMatrixRotationY(XMConvertToRadians(m_fTiltAccTime * TILT_PRECESS_DEG_SEC));

	const _float3 vScale = m_pTransformCom->Get_Scaled();

	m_pTransformCom->Set_State(STATE::RIGHT,
		XMVector3Normalize(matTilt.r[0]) * vScale.x);
	m_pTransformCom->Set_State(STATE::UP,
		XMVector3Normalize(matTilt.r[1]) * vScale.y);
	m_pTransformCom->Set_State(STATE::LOOK,
		XMVector3Normalize(matTilt.r[2]) * vScale.z);
}

CFaintEffect* CFaintEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFaintEffect* pInstance = new CFaintEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFaintEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFaintEffect::Clone(void* pArg)
{
	CFaintEffect* pInstance = new CFaintEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CFaintEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFaintEffect::Free()
{
	__super::Free();
}
