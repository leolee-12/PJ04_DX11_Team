#include "DroppedBubble.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"

CDroppedBubble::CDroppedBubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CAbility_Bubble{ pDevice, pContext }
{
}

CDroppedBubble::CDroppedBubble(const CDroppedBubble& Prototype)
	: CAbility_Bubble(Prototype)
{
}

HRESULT CDroppedBubble::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eCollLayer = COLLISION_LAYER::DROPPED_BUBBLE;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	SetUp_Collider_CallBack();

	return S_OK;
}

void CDroppedBubble::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_bCaptured)
	{
		Update_Captured(fTimeDelta);
		return;
	}

	m_fTimer += fTimeDelta;
	if (m_fTimer < s_fDeSpawnTime && m_fTimer >= s_fBlinkTime)
	{
		// TODO : Ability ¸ðµ¨¸¸ ±ô¹Ú±ô¹Ú
	}
	else if (m_fTimer >= s_fDeSpawnTime)
	{
		Despawn();
	}

}

void CDroppedBubble::Activate(const _float3& vPos)
{
	__super::Activate(vPos);

	m_bCaptured		= false;
	m_pCaptor		= nullptr;
	m_fPullSpeed	= 0.f;
	m_fScaleRatio	= 1.f;
}

_bool CDroppedBubble::Can_BeInhaled(const INHALE_QUERY& q) const
{
	return m_bAvailable && !m_bCaptured;
}

void CDroppedBubble::Be_Captured(CGameObject* pInhaler)
{
	if (m_bCaptured)
		return;

	m_bCaptured = true;
	m_pCaptor = pInhaler;

	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	m_fPullSpeed = 0.f;
	m_fScaleRatio = 1.f;

	// TODO : CCT ²ô±â
}

void CDroppedBubble::On_Swallowed()
{
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);

	m_pCaptor = nullptr;
	Despawn();
}

void CDroppedBubble::Update_Captured(_float fTimeDelta)
{
	if (nullptr == m_pCaptor)
		return;

	CTransform* pCapT = m_pCaptor->Get_Transform();
	_vector vMouth = pCapT->Get_State(STATE::POSITION)
		+ pCapT->Get_State(STATE::LOOK) * 0.6f
		+ pCapT->Get_State(STATE::UP) * 0.6f;

	_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float  fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= 0.5f)
	{
		On_Swallowed();
		return;
	}

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	m_pTransformCom->Set_State(STATE::POSITION,
		vSelf + XMVector3Normalize(vDir) * fMove);

	m_fScaleRatio += (s_fMinScaleRatio - m_fScaleRatio)
		* min(s_fShrinkLerp * fTimeDelta, 1.f);
	m_pTransformCom->Set_Scale(m_vBaseScale.x * m_fScaleRatio,
		m_vBaseScale.y * m_fScaleRatio,
		m_vBaseScale.z * m_fScaleRatio);
}

void CDroppedBubble::Despawn()
{
	if (!m_bActive)
		return;

	Set_Active(false);
	Set_RenderActive(false);
	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	Return_ToPool();

}

CDroppedBubble* CDroppedBubble::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDroppedBubble* pInstance = new CDroppedBubble(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDroppedBubble");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDroppedBubble::Clone(void* pArg)
{
	CDroppedBubble* pInstance = new CDroppedBubble(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDroppedBubble");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDroppedBubble::Free()
{
	__super::Free();
}
