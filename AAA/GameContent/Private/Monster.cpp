#include "Monster.h"
#include "Monster_StateMachine.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Monster_Brain_FSM.h"
#include "LevelDesign_LoadTypes.h"
#include "Sound_Handle.h"

#include "Collider.h"
#include "Controller.h"
#include "GameContent_const.h"
#include "Effect_Loader.h"

//#pragma warning(push, 0)
//#ifdef new
//#undef new
//#endif
//#include <PhysX/PxPhysicsAPI.h>
//#if defined(_DEBUG) && defined(DBG_NEW)
//#define new DBG_NEW
//#endif
//#pragma warning(pop)

CMonster::CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice, pContext }
{
}

CMonster::CMonster(const CMonster& Prototype)
	: CCharacter ( Prototype )
{
}

HRESULT CMonster::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMonster::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (nullptr != pArg)
	{
		const LD_OBJECT_DESC* pDesc = static_cast<const LD_OBJECT_DESC*>(pArg);
		if (!pDesc->strAIVariation.empty())
			Apply_AIVariation(pDesc->strAIVariation);
	}

	if (FAILED(Ready_Collider()))
		return E_FAIL;
	SetUp_Collider_CallBack();

	if (FAILED(Ready_PartObjects()))        
		return E_FAIL;
	if (FAILED(Ready_Movement()))           
		return E_FAIL;
	if (FAILED(Ready_AI()))                 
		return E_FAIL;
	if (FAILED(Ready_AnimEvents()))         
		return E_FAIL;

	return S_OK;
}

void CMonster::Priority_Update(_float fTimeDelta)
{
	if (!m_bActive) return;
	__super::Priority_Update(fTimeDelta);
}

void CMonster::Update(_float fTimeDelta)
{
	if (!m_bActive) return;
	Update_AI(fTimeDelta);
	__super::Update(fTimeDelta);
}

void CMonster::Late_Update(_float fTimeDelta)
{
	if (!m_bActive) return;
	__super::Late_Update(fTimeDelta);

	if (m_pInteractCollider)
	{
		m_pInteractCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		if(m_pInteractCollider->Is_Enabled())
			m_pGameInstance_Proxy->Add_DebugComponent(m_pInteractCollider);
#endif
	}

	if (m_pHurtBox)
	{
		{
			m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		if (m_pHurtBox->Is_Enabled())
			m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
		}
	}

	if (m_pProjectileBox && m_pProjectileBox->Is_Enabled())
	{
		m_pProjectileBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		if (m_pProjectileBox->Is_Enabled())
			m_pGameInstance_Proxy->Add_DebugComponent(m_pProjectileBox);
#endif
	}
}

void CMonster::On_Deserialized()
{
	if (nullptr != m_pMovement)
		m_pMovement->Sync_To_Controller();

	// 스폰 위치를 기억해뒀다가 사용
	XMStoreFloat3(&m_vBasePos, m_pTransformCom->Get_State(STATE::POSITION));
}

void CMonster::Set_Target(CGameObject* pTarget)
{
	m_BlackBoard.pTarget = pTarget;

	if (nullptr == pTarget)
	{
		m_BlackBoard.bCanSeeTarget = false;
		m_BlackBoard.fDistToTarget = FLT_MAX;
		m_BlackBoard.fDistToTargetXZ = FLT_MAX;
		m_BlackBoard.fHeightToTarget = 0.f;
		m_BlackBoard.vDirToTargetXZ = {};
	}
}

_bool CMonster::Can_BeInhaled(const INHALE_QUERY& q) const
{
	if (!Has_Trait(MT_INHALABLE))                              return false;
	if (m_pCaptor != nullptr)                                 return false;
	if (Has_Trait(MT_STRONG_INHALE_ONLY) && !q.bSuperInhale)  return false;
	return true;
}

void CMonster::Be_Captured(CGameObject* pInhaler)
{
	m_pCaptor = pInhaler;
	Change_State(MONSTER_STATE_TYPE::CAPTURED);
}

void CMonster::Add_MoveDir(const _float3& vWishDir)
{
	XMStoreFloat3(&m_vWishDir,
		XMLoadFloat3(&m_vWishDir) + XMLoadFloat3(&vWishDir));
}

void CMonster::Add_MoveDir(_fvector vWishDir)
{
	XMStoreFloat3(&m_vWishDir,
		XMLoadFloat3(&m_vWishDir) + vWishDir);
}

_bool CMonster::Has_MoveDir() const
{
	_vector vWishDir = XMLoadFloat3(&m_vWishDir);

	if (XMVector3Equal(vWishDir, XMVectorZero()))
		return false;

	return true;
}

void CMonster::Clear_MoveDir()
{
	XMStoreFloat3(&m_vWishDir, XMVectorZero());
}

_bool CMonster::Change_State(MONSTER_STATE_TYPE eNewState)
{
	if (nullptr == m_pStateMachine)
		return false;

	return m_pStateMachine->Change_State(eNewState);
}

_bool	CMonster::Has_State(MONSTER_STATE_TYPE eState) const
{
	if (nullptr == m_pStateMachine)
		return false;

	return m_pStateMachine->Has_State(eState);
}

MONSTER_STATE_TYPE	CMonster::Get_StateType() const
{
	if (nullptr == m_pStateMachine)
		return MONSTER_STATE_TYPE::IDLE;

	return m_pStateMachine->Get_StateType();
}

HRESULT CMonster::Ready_Collider()
{
	_float fRadius;
	_float3 vFootPos;
	XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));

	if ((fRadius = Get_InteractRadius()) != 0.f)
	{
		CCollider::COLLIDER_DESC ColliderDesc{};
		ColliderDesc.pOwner = this;
		//ColliderDesc.vCenter = vFootPos;
		ColliderDesc.vCenter = _float3{ 0,0,0 };
		ColliderDesc.fRadius = fRadius;

		m_pInteractCollider = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
			TEXT("InteractCol_Com"), &ColliderDesc);
		if (m_pInteractCollider == nullptr)
			return E_FAIL;

		m_pGameInstance_Proxy->Register_Collider(m_pInteractCollider, ETOUI(COLLISION_LAYER::MONSTER_D_RANGE));
	}

	CAPSULE_DESC Desc{};
	if (Get_HurtBoxDesc(Desc))
	{
		CCollider::COLLIDER_DESC HurtDesc{};
		HurtDesc.pOwner = this;
		HurtDesc.vCenter = Desc.vCenter;
		HurtDesc.fRadius = Desc.fRadius;
		HurtDesc.fHeight = Desc.fHeight;
		HurtDesc.vRadians = Desc.vRadians;

		m_pHurtBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
			TEXT("MonHurtBox_Com"), &HurtDesc);
		if (m_pHurtBox == nullptr)
			return E_FAIL;

		m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::MONSTER_HURT));

		CCollider::COLLIDER_DESC ProjDesc{};
		ProjDesc.pOwner = this;
		ProjDesc.vCenter = Desc.vCenter;
		ProjDesc.fRadius = Desc.fRadius;
		ProjDesc.fHeight = Desc.fHeight;
		ProjDesc.vRadians = Desc.vRadians;

		m_pProjectileBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
			TEXT("MonProjBox_Com"), &ProjDesc);
		if (m_pProjectileBox == nullptr)
			return E_FAIL;

		m_pProjectileBox->Set_Enabled(false);
		m_pGameInstance_Proxy->Register_Collider(m_pProjectileBox, ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE));
	}

	return S_OK;
}

void CMonster::SetUp_Collider_CallBack()
{
	if (m_pInteractCollider)
	{
		m_pInteractCollider->Set_OnEnter([this](CCollider* pOther) {
			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup())
				Set_Target(pOther->Get_Owner());
#ifdef _DEBUG
			char szBuf[128];
			sprintf_s(szBuf, "[Monster] Enter <- group %u\n", pOther->Get_RegisteredGroup());
			OutputDebugStringA(szBuf);
#endif // _DEBUG
			});
		m_pInteractCollider->Set_OnExit([this](CCollider* pOther) {
			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup())
				Set_Target(nullptr);
#ifdef _DEBUG
			char szBuf[128];
			sprintf_s(szBuf, "[Monster] Exit <- group %u\n", pOther->Get_RegisteredGroup());
			OutputDebugStringA(szBuf);
#endif // _DEBUG
			});
	}

	if (m_pHurtBox)
	{
		m_pHurtBox->Set_OnEnter([this](CCollider* pOther) {
			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup()
				&& Has_Trait(MT_BODYCHECK_DAMAGE))
			{
				_vector vAtkPos = pOther->Get_Owner()->Get_Transform()->Get_State(STATE::POSITION);
				ATTACK_INFO atk{};
				atk.fDamage = 1.f;
				atk.fKnockback = 8.f;
				XMStoreFloat3(&atk.vAttackerPos, vAtkPos);
				atk.pAttacker = pOther->Get_Owner();
				Damaged(atk);
#ifdef _DEBUG
				char szBuf[128];
				sprintf_s(szBuf, "[Monster] Hurt! HP %.0f/%.0f\n", m_fCurHP, m_fMaxHP);
				OutputDebugStringA(szBuf);
#endif
			}
		});
	}

	if (m_pProjectileBox)
	{
		m_pProjectileBox->Set_OnEnter([this](CCollider* pOther) {
			if (ETOUI(COLLISION_LAYER::MONSTER_HURT) != pOther->Get_RegisteredGroup())
				return;
			if (pOther->Get_Owner() == this)
				return;

			IDamageable* pVictim = dynamic_cast<IDamageable*>(pOther->Get_Owner());
			if (nullptr == pVictim)
				return;

			ATTACK_INFO atk{};
			atk.fDamage = s_fSpatDamage;
			atk.fKnockback = s_fSpatKnockback;
			XMStoreFloat3(&atk.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
			atk.pAttacker = this;
			pVictim->Damaged(atk);

			Despawn_Spat();   // 첫 명중 시 소멸(원작식)
			});
	}

	return;
}

HRESULT CMonster::Ready_Movement()
{
	_float3 vFootPos;
	XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));

	m_pController = Add_Component<CController>(TEXT("Com_Controller"),
		CController::Create(m_pDevice, m_pContext));
	if (nullptr == m_pController) return E_FAIL;

	CController::CONTROLLER_DESC ctrlDesc{};
	ctrlDesc.vFootPos = vFootPos;
	ctrlDesc.fRadius = Get_CapsuleRadius();
	ctrlDesc.fHeight = Get_CapsuleHeight();
	ctrlDesc.pOwner = this;
	if (FAILED(m_pController->Initialize(&ctrlDesc))) return E_FAIL;

	if (FAILED(Create_Movement())) return E_FAIL;

	m_pMovement->Set_Refs(m_pTransformCom, m_pController->Get_Raw());
	return S_OK;
}

HRESULT CMonster::Ready_AI()
{
	// 윤석현 수정
	m_pBrain = Create_Brain();
	if (nullptr == m_pBrain)
		return E_FAIL;

	if (Use_StateMachine())
	{
		m_pStateMachine = CMonster_StateMachine::Create(this);  // 초기 IDLE
		if (nullptr == m_pStateMachine)
		{
			Safe_Release(m_pBrain);
			return E_FAIL;
		}

		if (FAILED(Ready_State()))
		{
			Safe_Release(m_pStateMachine);
			Safe_Release(m_pBrain);
			return E_FAIL;
		}

		// 시작 IDLE 지정
		if (!Change_State(MONSTER_STATE_TYPE::IDLE))
		{
			Safe_Release(m_pStateMachine);
			Safe_Release(m_pBrain);
			return E_FAIL;
		}
	}

	return S_OK;
}

void CMonster::Check_AirborneReflex(_float fTimeDelta)
{
	if (nullptr == m_pStateMachine || nullptr == m_pMovement)
		return;

	if (!Has_State(MONSTER_STATE_TYPE::FALL))
		return;

	const MONSTER_STATE_TYPE eState = Get_StateType();
	if (eState == MONSTER_STATE_TYPE::FALL || eState == MONSTER_STATE_TYPE::LANDING ||
		eState == MONSTER_STATE_TYPE::CAPTURED || eState == MONSTER_STATE_TYPE::KNOCK_OUT ||
		eState == MONSTER_STATE_TYPE::KNOCK_BACK_DEATH)
	{
		m_fAirborneTimer = 0.f;
		return;
	}

	if (m_pMovement->Is_Bouncing())                         
	{
		m_fAirborneTimer = 0.f;
		return;
	}

	if (m_pMovement->Is_Grounded() || m_pMovement->Get_VerticalVelocity() >= 0.f)
	{
		m_fAirborneTimer = 0.f;
		return;
	}

	m_fAirborneTimer += fTimeDelta;
	if (m_fAirborneTimer >= s_fCoyoteTime)
		Change_State(MONSTER_STATE_TYPE::FALL);
}

CMonsterBrain* CMonster::Create_Brain()
{
	return nullptr; // 기본은 FSM
}

HRESULT CMonster::Create_Movement()
{
	m_pMovement = Add_Component<CMonster_Movement>(TEXT("Com_Movement"), CMonster_Movement::Create(m_pDevice, m_pContext));

	if (nullptr == m_pMovement)
		return E_FAIL;

	return S_OK;
}

HRESULT CMonster::Ready_State()
{
	return S_OK;
}

void CMonster::On_Damaged(const ATTACK_INFO& tInfo)
{
	m_LastHit = { tInfo.vAttackerPos, tInfo.fKnockback, tInfo.fDamage };

	//Play_OneShotSFX(L"CharaBasic_DamageReact_Normal.wav", 0.8f);

	// 몬스터 피격 이펙트
	const _float4* pCamLook = m_pGameInstance_Proxy->Get_CamLook();

	_float3 vFaceCam{};
	XMStoreFloat3(&vFaceCam, XMVectorNegate(XMLoadFloat4(pCamLook)));

	_float3	vPos{};
	XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));

	CEffect_Loader::GetInstance()->Spawn(L"CommonHit", Get_LevelIndex(),
		vPos, vFaceCam, _float3(0.f, 0.f, 0.f),
		nullptr);

	Change_State(MONSTER_STATE_TYPE::KNOCK_BACK);
}

void CMonster::On_Death(const ATTACK_INFO& tInfo)
{
	//  Movement 상태 안으로 이전
	m_LastHit = { tInfo.vAttackerPos, tInfo.fKnockback, tInfo.fDamage };

	if (tInfo.fDamage >= m_fMaxHP)
	{
		Change_State(MONSTER_STATE_TYPE::KNOCK_OUT);
	}
	else
	{
		Change_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH);					
	}
}

_bool CMonster::Block_Hit(const ATTACK_INFO& tInfo)
{
	if (Get_StateType() == MONSTER_STATE_TYPE::CAPTURED)
		return true;

	return false;
}

void CMonster::Perceive(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (nullptr == m_BlackBoard.pTarget)
	{
		m_BlackBoard.bCanSeeTarget = false;
		m_BlackBoard.fDistToTarget = FLT_MAX;
		m_BlackBoard.fDistToTargetXZ = FLT_MAX;
		m_BlackBoard.fHeightToTarget = 0.f;
		m_BlackBoard.vDirToTargetXZ = {};
		return;
	}

	CTransform* pTargetTransform = m_BlackBoard.pTarget->Get_Transform();
	if (nullptr == pTargetTransform)
	{
		m_BlackBoard.bCanSeeTarget = false;
		m_BlackBoard.fDistToTarget = FLT_MAX;
		m_BlackBoard.fDistToTargetXZ = FLT_MAX;
		m_BlackBoard.fHeightToTarget = 0.f;
		m_BlackBoard.vDirToTargetXZ = {};
		return;
	}

	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = pTargetTransform->Get_State(STATE::POSITION);
	_vector vToTarget = vTargetPos - vMyPos;
	_vector vToTargetXZ = XMVectorSetY(vToTarget, 0.f);

	_float fDistXZ = XMVectorGetX(XMVector3Length(vToTargetXZ));

	if (fDistXZ > 0.0001f)
	{
		XMStoreFloat3(
			&m_BlackBoard.vDirToTargetXZ,
			XMVector3Normalize(vToTargetXZ));
	}
	else 
		m_BlackBoard.vDirToTargetXZ = {};

	XMStoreFloat3(&m_BlackBoard.vTargetPos, vTargetPos);

	m_BlackBoard.fDistToTarget = XMVectorGetX(XMVector3Length(vToTarget));
	m_BlackBoard.fDistToTargetXZ = XMVectorGetX(XMVector3Length(vToTargetXZ));
	m_BlackBoard.fHeightToTarget = XMVectorGetY(vToTarget);

	m_BlackBoard.bCanSeeTarget = true;
	m_BlackBoard.vLastKnownPos = m_BlackBoard.vTargetPos;
}

_bool CMonster::Handle_SharedAnimEvent(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
	switch (static_cast<EANIM_EVENT>(e.iEventType))
	{
	case EANIM_EVENT::Sound:
	{
		if (ePhase != ANIM_EVENT_PHASE::POINT)
			return true;
		if (e.strParam.empty())
			return true;

		string strKey = e.strParam;
		const size_t iLen = strKey.size();
		if (iLen < 4 ||	
			strKey.compare(iLen - 4, 4, ".wav") != 0)
			strKey += ".wav";

		if (m_bSFX2D)
		{
			m_pGameInstance_Proxy->Play_SFX(StrToWstr(strKey).c_str(), e.vOffset.x);
		}
		else
		{
			_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
			m_pGameInstance_Proxy->Play_SFX3D(StrToWstr(strKey).c_str(), vPos, e.vOffset.x);
		}
		return true;
	}
	default:
		return false;		// 몬스터 고유 이벤트로 넘김
	}
}

void CMonster::Play_DeathFX()
{
	Play_OneShotSFX(L"CharaBasic_Dead.wav", 0.6f);

	const _float4* pCamLook = m_pGameInstance_Proxy->Get_CamLook();
	_float3 vFaceCam{};
	XMStoreFloat3(&vFaceCam,
		XMVectorNegate(XMLoadFloat4(pCamLook)));
	_float3 vPos{};
	XMStoreFloat3(&vPos,
		m_pTransformCom->Get_State(STATE::POSITION));

	CEffect_Loader::GetInstance()->Spawn(
		L"DespawnEffect", Get_LevelIndex(),
		vPos, vFaceCam, _float3(0.f, 0.f, 0.f), nullptr);
}

void CMonster::Play_ActionLoopSFX(const _tchar* pKey)
{
	m_ActionLoopSnd.Stop();
	m_ActionLoopSnd = m_pGameInstance_Proxy->Play_SFX_Loop(pKey);
}

void CMonster::Stop_ActionLoopSFX()
{
	m_ActionLoopSnd.Stop();
}

void CMonster::Play_OneShotSFX(const _tchar* pKey, _float fVolume, ESoundBus eBus)
{
	m_pGameInstance_Proxy->Play_SFX(pKey, fVolume, eBus);
}

void CMonster::Play_OneShotSFX3D(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume, ESoundBus eBus)
{
	m_pGameInstance_Proxy->Play_SFX3D(pSoundKey, vSoundPos, fVolume, eBus);
}

void CMonster::Enable_Controller(_bool bEnable)
{
	if (m_pController) m_pController->Set_Enabled(bEnable);
}

void CMonster::Enable_Colliders(_bool bEnable)
{
	if (m_pInteractCollider) m_pInteractCollider->Set_Enabled(bEnable);
	if (m_pHurtBox)          m_pHurtBox->Set_Enabled(bEnable);
}

void CMonster::On_Swallowed()
{
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EVT_SWALLOWED, &payload);
	m_pCaptor = nullptr;
	Set_Active(false);
}

void CMonster::Be_Spat(_fvector vPos, _fvector vDir, _float fSpeed)
{
	m_pCaptor = nullptr;
	Set_Active(true);                         

	CTransform* pT = m_pTransformCom;
	pT->Set_State(STATE::POSITION, vPos);
	pT->LookAt(XMVectorAdd(vPos, vDir));

	XMStoreFloat3(&m_vSpatVelocity, XMVector3Normalize(vDir) * fSpeed);

	Change_State(MONSTER_STATE_TYPE::SPAT);
}

void CMonster::Despawn_Spat()
{
	// TODO: 사운드, 이펙트 
	Play_DeathFX();

	Enable_ProjectileBox(false);
	m_vSpatVelocity = {};
	Set_Active(false);
}

void CMonster::Despawn()
{
	Play_DeathFX();

	Enable_Colliders(false);
	Enable_Controller(false);
	Set_Active(false);
}

void CMonster::Enable_ProjectileBox(_bool bEnable)
{
	if (m_pProjectileBox) m_pProjectileBox->Set_Enabled(bEnable);
}

void CMonster::Update_AI(_float fTimeDelta)
{
	// 이전 프레임 이동 요청 초기화
	Clear_MoveDir(); 

	// BlackBoard 갱신
	Perceive(fTimeDelta);	

	const _bool bEditMode = m_pGameInstance_Proxy->Is_EditMode();

	if (nullptr == m_pMovement)
		return;

	// Launched 물리 먼저
	if (m_pMovement->Is_Launched() && !bEditMode)
		m_pMovement->Update_Launched(fTimeDelta);

	// Brain이 상태 변경 판단
	if (nullptr != m_pBrain)
		m_pBrain->Decide(m_BlackBoard, fTimeDelta);

	// 현재 State 실행
	if (nullptr != m_pStateMachine)
		m_pStateMachine->Update_StateMachine(fTimeDelta);
	
	// STATE::FALL 로 전환하는 체크 함수
	Check_AirborneReflex(fTimeDelta);

	if (bEditMode)
	{
		m_pMovement->Sync_To_Controller();
		return;
	}

	if (Get_StateType() == MONSTER_STATE_TYPE::CAPTURED)
		return;

	if (Get_StateType() == MONSTER_STATE_TYPE::SPAT)
	{
		m_pMovement->Sync_To_Controller();
		return;
	}

	if (m_pMovement->Is_JumpArc())
	{
		m_pMovement->Update_JumpArc(fTimeDelta);
	}
	else if (!m_pMovement->Is_Launched())
	{
		if (Has_MoveDir())
		{
			_vector vDir = XMLoadFloat3(&m_vWishDir);
			m_pMovement->Move(vDir, fTimeDelta);
		}
		else
		{
			m_pMovement->Move(XMVectorZero(), fTimeDelta);
		}
	}
}

void CMonster::Free()
{
	Stop_ActionLoopSFX();

	Safe_Release(m_pBrain);
	Safe_Release(m_pStateMachine);
	__super::Free();
}
