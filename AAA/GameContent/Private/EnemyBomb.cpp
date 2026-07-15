#include "EnemyBomb.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"

CEnemyBomb::CEnemyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile_Bomb { pDevice , pContext }
{
}

CEnemyBomb::CEnemyBomb(const CEnemyBomb& Prototype)
	: CProjectile_Bomb ( Prototype ) 
{
}

_bool CEnemyBomb::Can_BeInhaled(const INHALE_QUERY& q) const
{
	return m_bAlive
		&& BOMB_STATE::CAPTURED != m_eState
		&& BOMB_STATE::HELD != m_eState;
}

void CEnemyBomb::Be_Captured(CGameObject* pInhaler)
{
	if (BOMB_STATE::CAPTURED == m_eState)
		return;

	m_pCaptor = pInhaler;
	Change_State(BOMB_STATE::CAPTURED);
}

void CEnemyBomb::On_Swallowed()
{
	m_pTransformCom->Set_Scale(
		m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);

	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);

	m_pCaptor = nullptr;
	Despawn();
}

HRESULT CEnemyBomb::Initialize(void* pArg)
{
	m_fSpeed = 25.f;
	m_fDamage = 2.f;
	m_fKnockback = 4.f;
	m_fHitRadius = 0.60f;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CEnemyBomb::Update(_float fTimeDelta)
{
	if (m_bAlive && BOMB_STATE::CAPTURED == m_eState)
	{
		Update_State(fTimeDelta);	// 물리 대신 당김 연출
		Update_FuseSocket();
		return;
	}

	__super::Update(fTimeDelta);
}

HRESULT CEnemyBomb::Ready_Visual()
{
	if (FAILED(__super::Ready_Visual()))
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC ad{};
	ad.pModel = m_pModelCom;
	ad.strDataFile = TEXT("../../Resources/CHJ/Monster/PoppyBrosJr/EnemyBomb/EnemyBomb_AnimEvents.json");
	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"),
		CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&ad)))
		return E_FAIL;

	return S_OK;
}

void CEnemyBomb::On_Activated()
{
	m_eState = BOMB_STATE::NONE;	// 풀 재사용 리셋
	m_pCaptor = nullptr;

	if (m_bCarried)
		Change_State(BOMB_STATE::HELD);
}

void CEnemyBomb::On_Launched()
{
	Change_State(BOMB_STATE::FLYING);
}

void CEnemyBomb::On_Bounce(_int iCount)
{
	if (iCount == 1)
		Change_State(BOMB_STATE::DANGER);
}

void CEnemyBomb::Change_State(BOMB_STATE eNext)
{
	if (m_eState == eNext)
		return;

	Exit_State(m_eState);
	m_eState = eNext;
	Enter_State(eNext);
}

void CEnemyBomb::Enter_State(BOMB_STATE eState)
{
	switch (eState)
	{
	case BOMB_STATE::HELD:
	{
		Reset_BombVisual();
		Play_BodyAnim(ANIM_FUSE, false);
		Start_Fuse();
		Pause_Fuse();
		Update_Socket();
		Spawn_FuseFx();
	}
		break;

	case BOMB_STATE::FLYING:
	{
		Reset_BombVisual();
		Play_BodyAnim(ANIM_FUSE, false);
		Start_Fuse();
		Spawn_FuseFx();
	}
		break;

	case BOMB_STATE::DANGER:
	{
		Play_DangerGlow();
		Resume_Fuse();
	}
		break;

	case BOMB_STATE::CAPTURED:
	{
		if (m_pHitBox)
			m_pHitBox->Set_Enabled(false);
		if (m_pController)
			m_pController->Set_Enabled(false);
		Pause_Fuse();

		m_fPullSpeed = 0.f;
		m_vBaseScale = m_pTransformCom->Get_Scaled();
		m_fScaleRatio = 1.f;
	}
		break;

	default:
		break;
	}
}

void CEnemyBomb::Update_State(_float fTimeDelta)
{
	switch (m_eState)
	{
	case BOMB_STATE::CAPTURED:
		Update_Captured(fTimeDelta);
		break;

	default:
		break;
	}
}

void CEnemyBomb::Exit_State(BOMB_STATE eState)
{
}

HRESULT CEnemyBomb::Ready_AnimEvents()
{
	if (nullptr == m_pAnimatorCom)
		return E_FAIL;

	m_pAnimatorCom->Set_EventCallback(
		[this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
		{
			switch (static_cast<EANIM_EVENT>(e.iEventType))
			{
			case EANIM_EVENT::SetBody:
				if (phase == ANIM_EVENT_PHASE::POINT)
					m_vGlow = _float3(e.vOffset.x, e.vOffset.y, e.vOffset.z);
				break;
			default:
				break;
			}
		});
	return S_OK;
}

void CEnemyBomb::Update_Captured(_float fTimeDelta)
{
	if (nullptr == m_pCaptor)
		return;

	CTransform* pCapT = m_pCaptor->Get_Transform();
	_vector vMouth = pCapT->Get_State(STATE::POSITION)
						+ pCapT->Get_State(STATE::LOOK) * 0.6f
						+ pCapT->Get_State(STATE::UP) * 0.6f;

	_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

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

	if (m_pAnimatorCom)
	{
		Apply_RollPose();		
		m_pAnimatorCom->Update(fTimeDelta);
	}
}

CEnemyBomb* CEnemyBomb::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnemyBomb* pInstance = new CEnemyBomb(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnemyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnemyBomb::Clone(void* pArg)
{
	CEnemyBomb* pInstance = new CEnemyBomb(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnemyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnemyBomb::Free()
{
	__super::Free();
}


