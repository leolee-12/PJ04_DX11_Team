#include "VanishEffect.h"

#include "MeshCommon.h"

CVanishEffect::CVanishEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
	Init_PropertyValue();
}

CVanishEffect::CVanishEffect(const CVanishEffect& Prototype)
	: CEffect_Container(Prototype)
{
	Init_PropertyValue();
}

HRESULT CVanishEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CVanishEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CVanishEffect::Update(_float fTimeDelta)
{
	Update_Move(fTimeDelta);
	__super::Update(fTimeDelta);
}

HRESULT CVanishEffect::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = true;
	tDesc.bUseTextureCom = false;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeSphereOriginal";

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Proto_SmokeSphereOriginal", &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeLowPoly";

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Prototype_Component_Model_SmokeLowPoly_1", &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Prototype_Component_Model_SmokeLowPoly_2", &tDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Prototype_Component_Model_SmokeLowPoly_3", &tDesc)))
		return E_FAIL;

	return S_OK;
}

void CVanishEffect::Update_Move(_float fTimeDelta)
{
	if (m_bIsPlay == false || m_fDuration <= Helper::fEpsilon ||
		fabsf(m_fVanishMoveSpeed) <= Helper::fEpsilon || fTimeDelta <= 0.f)
		return;

	_float fStartRatio = std::clamp(m_fVanishMoveStartRatio, 0.f, 1.f);
	_float fEndRatio = std::clamp(m_fVanishMoveEndRatio, 0.f, 1.f);

	if (fStartRatio > fEndRatio)
		std::swap(fStartRatio, fEndRatio);

	const _float fMoveStartTime = fStartRatio * m_fDuration;
	const _float fMoveEndTime = fEndRatio * m_fDuration;
	const _float fFrameStartTime = std::clamp(m_fAccTime, 0.f, m_fDuration);
	const _float fFrameEndTime = (std::min)(fFrameStartTime + fTimeDelta, m_fDuration);

	const _float fActiveStartTime = (std::max)(fFrameStartTime, fMoveStartTime);
	const _float fActiveEndTime = (std::min)(fFrameEndTime, fMoveEndTime);
	const _float fActiveDelta = (std::max)(0.f, fActiveEndTime - fActiveStartTime);

	if (fActiveDelta <= 0.f)
		return;

	const _float fMoveDuration = fMoveEndTime - fMoveStartTime;
	if (fMoveDuration <= Helper::fEpsilon)
		return;

	const _float fLocalStartRatio = (fActiveStartTime - fMoveStartTime) / fMoveDuration;
	const _float fLocalEndRatio = (fActiveEndTime - fMoveStartTime) / fMoveDuration;
	const _float fLocalMidRatio = (fLocalStartRatio + fLocalEndRatio) * 0.5f;

	const _float fStartSpeedScale = 1.f - Helper::FloatSmoothStep(0.f, 1.f, fLocalStartRatio);
	const _float fMidSpeedScale = 1.f - Helper::FloatSmoothStep(0.f, 1.f, fLocalMidRatio);
	const _float fEndSpeedScale = 1.f - Helper::FloatSmoothStep(0.f, 1.f, fLocalEndRatio);
	const _float fSmoothedDelta = fActiveDelta *
		(fStartSpeedScale + 4.f * fMidSpeedScale + fEndSpeedScale) / 6.f;

	_vector vUp = m_pTransformCom->Get_State(STATE::UP);
	if (XMVectorGetX(XMVector3LengthSq(vUp)) <= Helper::fEpsilon)
		return;

	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	vPosition += XMVector3Normalize(vUp) * m_fVanishMoveSpeed * fSmoothedDelta;
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vPosition, 1.f));
}

void CVanishEffect::Init_PropertyValue()
{
	m_fVanishMoveSpeed = 0.f;
	m_fVanishMoveStartRatio = 0.f;
	m_fVanishMoveEndRatio = 1.f;
}

CVanishEffect* CVanishEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVanishEffect* pInstance = new CVanishEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CVanishEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CVanishEffect::Clone(void* pArg)
{
	CVanishEffect* pInstance = new CVanishEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CVanishEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVanishEffect::Free()
{
	__super::Free();
}