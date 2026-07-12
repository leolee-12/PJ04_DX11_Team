#include "EnemyBomb.h"
#include "GameInstance.h"
#include "Projectile_Movement.h"
#include "GameContrnt_Events.h"
#include "Effect_Loader.h"

CEnemyBomb::CEnemyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile_Bomb { pDevice , pContext }
{
	m_fSpeed = 25.f;
	m_fDamage = 2.f;
	m_fKnockback = 4.f;
	m_fHitRadius = 0.60f;
}

CEnemyBomb::CEnemyBomb(const CEnemyBomb& Prototype)
	: CProjectile_Bomb ( Prototype ) 
{
}

_bool CEnemyBomb::Can_BeInhaled(const INHALE_QUERY& q) const
{
	return m_bAlive && !m_bCaptured && !m_bCarried;
}

void CEnemyBomb::Be_Captured(CGameObject* pInhaler)
{
	if (m_bCaptured)
		return;

	m_bCaptured = true;
	m_pCaptor = pInhaler;

	if (m_pHitBox)
		m_pHitBox->Set_Enabled(false);     // 피격 off
	if (m_pController)
		m_pController->Set_Enabled(false);
	if (m_pAnimatorCom)
		m_pAnimatorCom->Pause_Mask(1);		// 도화선 애니메이션 멈추기

	m_fPullSpeed = 0.f;
	m_vBaseScale = m_pTransformCom->Get_Scaled();
	m_fScaleRatio = 1.f;
}

void CEnemyBomb::On_Swallowed()
{
	m_pTransformCom->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);

	SWALLOW_EVENT payload{ this };          
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);

	if (m_pFuseFx)
	{
		m_pFuseFx->EffectContainer_StopAfterEmission();

		m_pFuseFx = nullptr;
	}

	m_pCaptor = nullptr;
	Despawn();
}

void CEnemyBomb::Despawn()
{
	if (m_pFuseFx)
	{
		m_pFuseFx->EffectContainer_StopAfterEmission();

		m_pFuseFx = nullptr;
	}

	__super::Despawn();
}

void CEnemyBomb::Update(_float fTimeDelta)
{
	if (m_bAlive && m_bCaptured)		// 흡입 중 : 물리/폭발/피격 off
	{
		Update_Captured(fTimeDelta);	// 흡입 로직만
		Update_FuseSocket();
		return;
	}

	__super::Update(fTimeDelta);
	Update_FuseSocket();
}

HRESULT CEnemyBomb::Ready_Visual()
{
	if (FAILED(__super::Ready_Visual()))
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));

	if (nullptr == m_pModelCom)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC ad{};
	ad.pModel = m_pModelCom;
	ad.strDataFile = TEXT("../../Resources/CHJ/Monster/PoppyBrosJr/EnemyBomb/EnemyBomb_AnimEvents.json");
	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&ad)))
		return E_FAIL;

	return S_OK;
}

void CEnemyBomb::On_Activated()
{
	if (nullptr == m_pAnimatorCom)
		return;

	m_bCaptured = false;
	m_fRollAngle = 0.f;
	m_vGlow = { 0.f, 0.f, 0.f };
	m_pAnimatorCom->Clear_Overlay(1);

	m_pAnimatorCom->Play("FuseBurning", false, true);		// 크래쉬 안나게 설정

	// Overlay Animation 
	CAnimator::LAYER_PLAY_INFO LayerInfo{};
	LayerInfo.iSlot = 1;
	LayerInfo.tAnim.strAniName = "FuseBurning";
	LayerInfo.tAnim.bLoop = false;		// 해당 애니메이션 끝나면 수명 끝이므로 false
	LayerInfo.tAnim.bRestart = true;
	LayerInfo.tAnim.fSpeed = 1.f;
	LayerInfo.Roots = { "EffectL" };

	m_pAnimatorCom->Apply_Overlay(LayerInfo);

	if (m_bCarried)
	{
		m_pAnimatorCom->Pause_Mask(1);
		Update_Socket();
	}

	if (nullptr == m_pFuseBone)
		m_pFuseBone = m_pModelCom->Get_BoneMatrixPtr("EffectL");

	Update_FuseSocket();

	if (nullptr == m_pFuseFx)
	{
		CEffect_Loader::GetInstance()->Spawn(
			L"BombFuseEffect", Get_LevelIndex(),
			_float3(0.f, 0.f, 0.f),
			_float3(0.f, 0.f, 0.f),
			_float3(0.f, 0.f, 0.f),
			&m_matFuseWorld, &m_pFuseFx);
	}
}

void CEnemyBomb::On_Bounce(_int iCount)
{
	// 첫 착지만
	if (iCount != 1 || nullptr == m_pAnimatorCom)
		return;

	// Base Animation 
	CAnimator::ANI_PLAY_INFO AniInfo{};
	AniInfo.strAniName = "DangerGlow";
	AniInfo.bLoop = true;
	AniInfo.fSpeed = 2.0f;

	m_pAnimatorCom->Play(&AniInfo);			// Base 애니메이션 설정
	m_pAnimatorCom->Resume_Mask(1);
}

void CEnemyBomb::On_Explode()
{
	_float3 vPos{};
	XMStoreFloat3(&vPos,
		m_pTransformCom->Get_State(STATE::POSITION));

	m_pGameInstance_Proxy->Play_SFX3D(L"CharaPoppyBrosJr_BombExplode.wav", XMLoadFloat3(&vPos));

	CEffect_Loader::GetInstance()->Spawn(
		L"BombExplosion", Get_LevelIndex(),
		vPos, _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
		nullptr);

	if (m_pFuseFx)
	{
		m_pFuseFx->EffectContainer_StopAfterEmission();
		//m_pFuseFx->EffectContainer_Stop();

		m_pFuseFx = nullptr;
	}

	Despawn();
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
			{
				if (phase == ANIM_EVENT_PHASE::POINT)
				{
					m_vGlow = _float3(e.vOffset.x, e.vOffset.y, e.vOffset.z);
				}
				break;
			}
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
		m_pAnimatorCom->SetBoneRotation("RotL", m_fRollAngle, XMLoadFloat3(&m_vRollAxis));		// 회전 유지
		m_pAnimatorCom->Update(fTimeDelta);
	}
}

void CEnemyBomb::Update_FuseSocket()
{
	if (!m_pFuseBone)	return;
	_matrix matBoneWorld =
		XMLoadFloat4x4(m_pFuseBone) *
		XMLoadFloat4x4(Get_Transform()->Get_WorldMatrixPtr());

	_matrix matSocket = XMMatrixIdentity();   // 회전=월드 정렬
	matSocket.r[3] = matBoneWorld.r[3];

	XMStoreFloat4x4(&m_matFuseWorld, matSocket);
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


