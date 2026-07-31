#include "DroppedBubble.h"
#include "GameInstance.h"
#include "GameContent_Events.h"
#include "Controller.h"
#include "PhysX/characterkinematic/PxController.h"

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

	if (FAILED(Ready_Controller()))
		return E_FAIL;

	return S_OK;
}

void CDroppedBubble::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Update(fTimeDelta);

	if (m_bCaptured)
	{
		Update_Captured(fTimeDelta);
		return;
	}

	Update_Movement(fTimeDelta);

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

	if (m_pController)
	{
		m_pController->Set_Enabled(true);
		m_pController->Set_Solid(false);
		m_pController->Set_FootPosition(XMLoadFloat3(&vPos) - XMVectorSet(0.f, s_fFootOffset, 0.f, 0.f));
	}

	m_vVelocity = { 0.f, 0.f, 0.f };
	m_bIsGrounded = false;
	m_iBounceCount = 0;
	m_fBobPhase = vPos.x * 0.7f + vPos.z * 1.3f;
}

void CDroppedBubble::Launch(const _float3& vDir)
{
	_vector v = XMLoadFloat3(&vDir);
	if (XMVectorGetX(XMVector3LengthSq(v)) > 1e-4f)
	{
		v = XMVector3Normalize(v);
		_float3 d{};
		XMStoreFloat3(&d, v);
		m_vVelocity.x = d.x * s_fLaunchSpeed;
		m_vVelocity.y = d.y * s_fLaunchSpeed
			+ s_fPopUpSpeed;
		m_vVelocity.z = d.z * s_fLaunchSpeed;
	}
	else
	{
		m_vVelocity = { 0.f, s_fPopUpSpeed, 0.f };
	}

	m_bIsGrounded = false;
	m_iBounceCount = 0;
	On_Launched();
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

	m_vVelocity = { 0.f, 0.f, 0.f };

	if (m_pCollider)
		m_pCollider->Set_Enabled(false);
	if (m_pController)
		m_pController->Set_Enabled(false);

	m_fPullSpeed = 0.f;
	m_fScaleRatio = 1.f;

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

	m_bAvailable = false;

	Set_Active(false);
	Set_RenderActive(false);

	m_vVelocity = { 0.f, 0.f, 0.f };

	if (m_pController)
		m_pController->Set_Enabled(false);

	if (m_pCollider)
		m_pCollider->Set_Enabled(false);


	Return_ToPool();

}

void CDroppedBubble::SetUp_Collider_CallBack()
{
	if (nullptr == m_pCollider)
		return;

	m_pCollider->Set_OnStay([this](CCollider* pOther)
	{
		if (ETOUI(COLLISION_LAYER::PLAYER_HURT)
			!= pOther->Get_RegisteredGroup())
			return;
		if (m_bCaptured || !m_bAvailable)
			return;

		_vector vFrom = pOther->Get_Owner()
			->Get_Transform()->Get_State(STATE::POSITION);
		_vector vTo =
			m_pTransformCom->Get_State(STATE::POSITION);

		_vector vDir = XMVectorSetY(vTo - vFrom, 0.f);
		if (XMVectorGetX(XMVector3LengthSq(vDir)) < 1e-6f)
			return;

		_float3 d{};
		XMStoreFloat3(&d, XMVector3Normalize(vDir));
		m_vVelocity.x = d.x * s_fPushSpeed;
		m_vVelocity.z = d.z * s_fPushSpeed;
	});
}

HRESULT CDroppedBubble::Ready_Controller()
{
	CController::CONTROLLER_DESC cd{};
	cd.pOwner = this;
	cd.fRadius = 1.25f;
	cd.fHeight = 0.01f;
	cd.vFootPos = { 0.f, 0.f, 0.f };

	m_pController = Add_Component<CController>(TEXT("Com_BubbleController"), CController::Create(m_pDevice, m_pContext));
	if (nullptr == m_pController)
		return E_FAIL;
	if (FAILED(m_pController->Initialize(&cd)))
		return E_FAIL;

	m_pController->Set_Enabled(false);

	return S_OK;
}

void CDroppedBubble::Update_Movement(_float fTimeDelta)
{
	if (nullptr == m_pController)
		return;

	m_vVelocity.y += s_fGravity * fTimeDelta;

	m_fBobPhase += fTimeDelta;

	_float fSwayT = m_fBobPhase * s_fSwayFreq;
	_float fStep = s_fSwayAccel * fTimeDelta;
	m_vVelocity.x += fStep * sinf(fSwayT);
	m_vVelocity.z += fStep * cosf(fSwayT * 0.77f);

	_float k = 1.f - s_fAirDrag * fTimeDelta;
	if (k < 0.f)
		k = 0.f;
	m_vVelocity.x *= k;
	m_vVelocity.y *= k;
	m_vVelocity.z *= k;

	_vector vDisp =
		XMLoadFloat3(&m_vVelocity) * fTimeDelta;

	_uint iFlags =
		m_pController->Move(vDisp, 0.001f, fTimeDelta);

	m_pTransformCom->Set_State(STATE::POSITION,
		m_pController->Get_FootPosition()
		+ XMVectorSet(0.f, s_fFootOffset, 0.f, 0.f));

	using PXCF = physx::PxControllerCollisionFlag;

	const _bool bDown = (iFlags & PXCF::eCOLLISION_DOWN) != 0;

	if (bDown && m_vVelocity.y < 0.f)
	{
		if (!m_bIsGrounded)
		{
			m_bIsGrounded = true;
			On_Rest();
		}
		m_vVelocity.y = s_fHoverKick;
	}
}

void CDroppedBubble::On_Launched()
{
}

void CDroppedBubble::On_Bounce(_int iCount)
{
}

void CDroppedBubble::On_Rest()
{
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
