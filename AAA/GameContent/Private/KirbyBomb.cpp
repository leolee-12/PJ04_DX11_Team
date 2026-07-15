#include "KirbyBomb.h"

#include "GameContent_const.h"
#include "Projectile_Movement.h"

#include "Effect_Loader.h"

CKirbyBomb::CKirbyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile_Bomb{ pDevice , pContext }
{
}

CKirbyBomb::CKirbyBomb(const CKirbyBomb& Prototype)
	: CProjectile_Bomb(Prototype)
{
}

HRESULT CKirbyBomb::Initialize_Prototype()
{
	return S_OK;;
}

HRESULT CKirbyBomb::Initialize(void* pArg)
{
	// Launch 초기 속도
	m_fSpeed = 25.f;
	m_fDamage = 2.f;
	m_fKnockback = 4.f;
	m_fHitRadius = 0.60f;

	// Transform, Movement, HitBox, Shader 생성 
	// Model Animator 생성
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;	

	m_pAnimatorCom->Play("FuseBurning", false, true);

	return S_OK;;
}

void CKirbyBomb::Update(_float fTimeDelta)
{
	if (!m_bAlive)
		return;

	__super::Update(fTimeDelta);

	Update_FuseSocket();
}

HRESULT CKirbyBomb::Ready_Visual()
{
	if (FAILED(__super::Ready_Visual()))
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
	if (m_pModelCom == nullptr)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC tDesc{};
	tDesc.pModel = m_pModelCom;
	//tDesc.strDataFile = TEXT("");
	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (m_pAnimatorCom == nullptr || FAILED(m_pAnimatorCom->Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CKirbyBomb::On_Activated()
{
	if (nullptr == m_pAnimatorCom)
		return;

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

	if (m_pFuseBone == nullptr)
		m_pFuseBone = m_pModelCom->Get_BoneMatrixPtr("EffectL");

	Update_FuseSocket();

	if (m_pFuseFx == nullptr)
	{
		CEffect_Loader::GetInstance()->Spawn(L"BombFuseEffect", Get_LevelIndex(),
			_float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
			&m_matFuseWorld, &m_pFuseFx);
	}
}

void CKirbyBomb::On_Bounce(_int iCount)
{
	if (iCount != 1 || nullptr == m_pAnimatorCom)
		return;

	CAnimator::ANI_PLAY_INFO AniInfo{};
	AniInfo.strAniName = "DangerGlow";
	AniInfo.bLoop = true;
	AniInfo.fSpeed = 2.f;

	m_pAnimatorCom->Play(&AniInfo);
	m_pAnimatorCom->Resume_Mask(1);
}

void CKirbyBomb::On_Explode()
{
	_float3 vPos{};
	XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));

	CEffect_Loader::GetInstance()->Spawn(L"BombExplosion", Get_LevelIndex(),
		vPos, _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f));

	if (m_pFuseFx)
	{
		m_pFuseFx->EffectContainer_StopAfterEmission();
		m_pFuseFx = nullptr;
	}

	Despawn();
}

void CKirbyBomb::Update_FuseSocket()
{
	if (!m_pFuseBone)
		return;

	_matrix matWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	_matrix matBoneWorld = XMLoadFloat4x4(m_pFuseBone) * matWorld;

	_matrix matSocket = XMMatrixIdentity();
	matSocket.r[3] = matBoneWorld.r[3];

	XMStoreFloat4x4(&m_matFuseWorld, matSocket);
}

void CKirbyBomb::Despawn()
{
	if (m_pFuseFx)
	{
		m_pFuseFx->EffectContainer_StopAfterEmission();
		m_pFuseFx = nullptr;
	}

	__super::Despawn();  // Kill
}

HRESULT CKirbyBomb::Ready_AnimEvents()
{
	return S_OK;
}

HRESULT CKirbyBomb::Ready_HitBox()
{
	CCollider::COLLIDER_DESC desc{};
	desc.pOwner = this;
	desc.fHeight = m_fHitHeight;
	desc.fRadius = m_fHitRadius;
	desc.vCenter = m_vCenterOffset;
	desc.vRadians = m_vRadians;

	m_pHitBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
		TEXT("Com_HitBox"), &desc);
	if (m_pHitBox == nullptr)
		return E_FAIL;

	m_pHitBox->Set_OnEnter([this](CCollider* pOther)
		{
			if (!m_bAlive)
				return;

			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
				return;

			if (auto* pVictim = dynamic_cast<IDamageable*>(pOther->Get_Owner()))
			{
				ATTACK_INFO tAttackInfo{};
				tAttackInfo.eHitType = HIT_TYPE::BOMB;
				tAttackInfo.fDamage = m_fDamage;
				tAttackInfo.fKnockback = m_fKnockback;
				XMStoreFloat3(&tAttackInfo.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
				tAttackInfo.pAttacker = this;
				pVictim->Damaged(tAttackInfo);
			}

			On_Impact();
		}
	);

	m_pHitBox->Set_Enabled(false);
	m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::PLAYER_HIT));

	return S_OK;
}

CKirbyBomb* CKirbyBomb::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKirbyBomb* pInstance = new CKirbyBomb(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKirbyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CKirbyBomb::Clone(void* pArg)
{
	CKirbyBomb* pInstance = new CKirbyBomb(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CKirbyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKirbyBomb::Free()
{
	__super::Free();
}