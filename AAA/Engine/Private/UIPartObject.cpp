#include "UIPartObject.h"
#include "GameInstance.h"

CUIPartObject::CUIPartObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject(pDevice, pContext)
{
}

CUIPartObject::CUIPartObject(const CUIPartObject& Prototype)
	: CUIObject(Prototype)
{
}

void CUIPartObject::Play_Bounce(const _float2& vDirection, _float fDistance, _float fDuration, _float fWaveCount, _float fDamping)
{
	if (!m_pTransformCom)
		return;

	if (fDistance <= 0.f || fDuration <= FLT_EPSILON)
		return;

	_vector vDir = XMLoadFloat2(&vDirection);

	if (XMVectorGetX(XMVector2LengthSq(vDir)) <= FLT_EPSILON)
		return;

	vDir = XMVector2Normalize(vDir);

	if (m_Bounce.bPlaying)
		Stop_Bounce(true);

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

	m_Bounce.vBasePos =
	{
		XMVectorGetX(vPos),
		XMVectorGetY(vPos)
	};

	m_Bounce.fBaseZ = XMVectorGetZ(vPos);

	XMStoreFloat2(&m_Bounce.vDirection, vDir);

	m_Bounce.fDistance = fDistance;
	m_Bounce.fDuration = fDuration;
	m_Bounce.fElapsed = 0.f;
	m_Bounce.fWaveCount = max(0.1f, fWaveCount);
	m_Bounce.fDamping = max(0.f, fDamping);
	m_Bounce.bPlaying = true;
}

void CUIPartObject::Stop_Bounce(_bool bRestoreBase)
{
	if (!m_pTransformCom)
		return;

	if (m_Bounce.bPlaying && bRestoreBase)
	{
		m_pTransformCom->Set_State(
			STATE::POSITION,
			XMVectorSet(
				m_Bounce.vBasePos.x,
				m_Bounce.vBasePos.y,
				m_Bounce.fBaseZ,
				1.f));
	}

	m_Bounce.Reset();
}

HRESULT CUIPartObject::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CUIPartObject::Initialize(void* pArg)
{
	if (pArg)
	{
		auto pDesc = static_cast<UI_PARTOBJECT_DESC*>(pArg);
		m_pParentMatrix = pDesc->pParentMatrix;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CUIPartObject::Priority_Update(_float fTimeDelta)
{
}

void CUIPartObject::Update(_float fTimeDelta)
{
	Update_Bounce(fTimeDelta);
}

void CUIPartObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CUIPartObject::Render()
{
	return S_OK;
}

void CUIPartObject::Set_ParentMatrix(const _float4x4* pParentMatrix)
{
	m_pParentMatrix = pParentMatrix;
}

void CUIPartObject::Compute_CombinedWorldMatrix(_fmatrix ChildMatrix)
{
	XMStoreFloat4x4(&m_CombinedWorldMatrix, ChildMatrix * XMLoadFloat4x4(m_pParentMatrix));
}

void CUIPartObject::Update_Bounce(_float fTimeDelta)
{
	if (!m_Bounce.bPlaying || !m_pTransformCom)
		return;

	m_Bounce.fElapsed += fTimeDelta;

	_float fT = m_Bounce.fElapsed / m_Bounce.fDuration;

	if (fT >= 1.f)
	{
		Stop_Bounce(true);
		return;
	}

	fT = max(0.f, min(1.f, fT));

	const _float fWave =
		sinf(XM_PI * m_Bounce.fWaveCount * fT);

	const _float fEnvelope =
		(m_Bounce.fDamping <= 0.f)
		? 1.f
		: powf(1.f - fT, m_Bounce.fDamping);

	const _float fOffset =
		m_Bounce.fDistance * fWave * fEnvelope;

	const _float2 vPos =
	{
		m_Bounce.vBasePos.x + m_Bounce.vDirection.x * fOffset,
		m_Bounce.vBasePos.y + m_Bounce.vDirection.y * fOffset
	};

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(vPos.x, vPos.y, m_Bounce.fBaseZ, 1.f));
}

void CUIPartObject::Free()
{
	__super::Free();
}
