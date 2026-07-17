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

#include "Monster_State_Flatten.h"

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
	m_iCullPhase = rand() % 8;

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

	Compute_SpatPivot();

	return S_OK;
}

void CMonster::Priority_Update(_float fTimeDelta)
{
	if (!m_bActive) return;
	__super::Priority_Update(fTimeDelta);

	if (m_fBodyCheckOffTime > 0.f)
	{
		m_fBodyCheckOffTime -= fTimeDelta;
		if (m_fBodyCheckOffTime <= 0.f)
		{
			m_fBodyCheckOffTime = 0.f;
			if (m_bHadBodyCheckTrait)
				m_TraitFlags |= MT_BODYCHECK_DAMAGE;
		}
	}
}

void CMonster::Update(_float fTimeDelta)
{
	if (!m_bActive) return;

	fTimeDelta = Filter_TimeDelta(fTimeDelta);
	Update_CullGrade(fTimeDelta);
#ifdef _DEBUG
	if (m_pGameInstance_Proxy->Is_EditMode())
	{
		if (m_pMovement) m_pMovement->Sync_To_Controller();
		return;
	}
#endif

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

_bool CMonster::Change_State(MONSTER_STATE_TYPE eNewState, _bool bStateReenter)
{
	if (nullptr == m_pStateMachine)
		return false;

	return m_pStateMachine->Change_State(eNewState, bStateReenter);
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
				atk.fDamage = 10.f;
				atk.fKnockback = 8.f;
				atk.eHitType = HIT_TYPE::BODY_CONTACT;
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
		eState == MONSTER_STATE_TYPE::KNOCK_BACK_DEATH || eState == MONSTER_STATE_TYPE::FLATTEN)
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
	// 공통 FLATTEN 상태
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FLATTEN, CMonster_State_Flatten::Create())))
		return E_FAIL;

	return S_OK;
}

void CMonster::On_Damaged(const ATTACK_INFO& tInfo)
{
	if (tInfo.eHitType == HIT_TYPE::CAR_BOOSTER_HIT)
	{
		if (m_pMovement->Is_Grounded())
			Change_State(MONSTER_STATE_TYPE::FLATTEN);
		else
			Despawn();

		return;
	}

	if (tInfo.eHitType != HIT_TYPE::BODY_CONTACT)
		Open_BodyCheckBlock();

	m_LastHit = { tInfo.vAttackerPos, tInfo.fKnockback, tInfo.fDamage };

	// 몬스터 피격 이펙트
	const _float4* pCamLook = m_pGameInstance_Proxy->Get_CamLook();

	_float3 vFaceCam{};
	XMStoreFloat3(&vFaceCam, XMVectorNegate(XMLoadFloat4(pCamLook)));

	_float3	vPos{};
	XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));

	CEffect_Loader::GetInstance()->Spawn(L"CommonHit", Get_LevelIndex(),
		vPos, vFaceCam, _float3(0.f, 0.f, 0.f),
		nullptr);

	Change_State(MONSTER_STATE_TYPE::KNOCK_BACK, true);
}

void CMonster::On_Death(const ATTACK_INFO& tInfo)
{
	if (tInfo.eHitType != HIT_TYPE::BODY_CONTACT)
		Open_BodyCheckBlock();

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

void CMonster::Update_CullGrade(_float fTimeDelta)
{
	if (m_fCullDist <= 0.f)
	{
		m_CullState = {};
		m_CullState.fAnimDt = fTimeDelta;
		return;
	}

	BoundingSphere tSphere{};
	XMStoreFloat3(&tSphere.Center,
		m_pTransformCom->Get_State(STATE::POSITION));
	tSphere.Radius = Get_CapsuleHeight() * 0.5f
		+ Get_CapsuleRadius();

	const CULLING_FADE_RESULT tFade =
		m_pGameInstance_Proxy->Evaluate_DistanceFade(
			tSphere, m_fCullDist, m_fFadeRange);

	if (FLT_MAX == tFade.fBoundaryDistance)
	{   // fail-open: treat as FULL
		m_CullState = {};
		m_CullState.fAnimDt = fTimeDelta;
		return;
	}

	m_CullState.fDissolve = tFade.fDissolve;
	m_CullState.bRenderCull = tFade.bCulled;

	if (m_pGameInstance_Proxy->Is_EditMode())
	{   // keep visibility fresh, no dt accum
		m_fAnimAccum = 0.f;
		m_CullState.bAnimTick = false;
		m_CullState.fAnimDt = 0.f;
		return;
	}

	const _uint iPeriod =
		Calc_AnimPeriod(tFade.fBoundaryDistance);

	m_fAnimAccum += fTimeDelta;
	++m_iCullFrame;

	m_CullState.bAnimTick = (iPeriod <= 1)
		|| (0 == ((m_iCullFrame + m_iCullPhase) % iPeriod));

	if (m_CullState.bAnimTick)
	{
		m_CullState.fAnimDt = m_fAnimAccum;
		m_fAnimAccum = 0.f;
	}
}

_uint CMonster::Calc_AnimPeriod(_float fDist) const
{
	const _uint* P = s_bUseAnimURO ? s_iPeriodB : s_iPeriodA;

	if (m_CullState.bRenderCull)		return P[3];
	if (fDist >= m_fBandQuarter)		return P[2];
	if (fDist >= m_fBandHalf)			return P[1];
	return P[0];
}

_bool CMonster::Handle_SoundAnimEvent(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
	switch (static_cast<EANIM_EVENT>(e.iEventType))
	{
	case EANIM_EVENT::Sound:
	{
		if (ePhase != ANIM_EVENT_PHASE::POINT)
			return true;

		if (e.strParam.empty())
			return true;

		if (m_bSFX2D)
		{
			m_pGameInstance_Proxy->Play_SFX(StrToWstr(e.strParam).c_str(), e.vOffset.x);
		}
		else
		{
			_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
			m_pGameInstance_Proxy->Play_SFX3D(StrToWstr(e.strParam).c_str(), vPos, e.vOffset.x);
		}
		return true;
	}
	default:
		return false;		// 몬스터 고유 이벤트로 넘김
	}
}

_bool CMonster::Handle_FxAnimEvent(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
	if (static_cast<EANIM_EVENT>(e.iEventType) != EANIM_EVENT::Fx)
		return false;

	if (ePhase == ANIM_EVENT_PHASE::TICK)
		return true;

	const _wstring strFx = StrToWstr(e.strParam);
	if (strFx.empty())
		return true;

	_float3 vPos{}, vLook{ 0.f, 0.f, 1.f }, vRotDeg{};
	const _float4x4* vAncorMat = { nullptr };
	switch (e.iIntParam)
	{
	case 1:     vPos = e.vOffset;														break;
	case 2:    	vLook = e.vOffset;														break;
	case 3:    	XMStoreFloat3(&vLook, m_pTransformCom->Get_State(STATE::LOOK));			break;
	case 4:     vRotDeg = e.vOffset;													break;
	case 5:    	XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));		break;
	case 6:		vAncorMat = Get_FxParentMatrix(strFx);									break;
	case 7:		// 몬스터 기준 월드 스폰: 위치=월드행렬×오프셋, LOOK=몬스터 LOOK
	{
		XMStoreFloat3(&vPos, XMVector3TransformCoord(XMLoadFloat3(&e.vOffset),
			XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
		XMStoreFloat3(&vLook, m_pTransformCom->Get_State(STATE::LOOK));
	}
		break;
	default:
		break;
	}

	auto pLoader = CEffect_Loader::GetInstance();

	if (ePhase == ANIM_EVENT_PHASE::POINT)
	{   
		pLoader->Spawn(strFx, Get_LevelIndex(),
			vPos, vLook, vRotDeg, vAncorMat);
		return true;
	}

	if (ePhase == ANIM_EVENT_PHASE::BEGIN)
	{
		FX_HANDLE& hSlot = m_Effects[strFx];
		if (pLoader->Is_Current(hSlot))
			hSlot.p->EffectContainer_StopAfterEmission();
		hSlot.Clear();

		pLoader->Spawn(strFx, Get_LevelIndex(),
			vPos, vLook, vRotDeg, Get_FxParentMatrix(strFx),
			nullptr, &hSlot);
	}
	else if (ePhase == ANIM_EVENT_PHASE::END)
	{
		auto it = m_Effects.find(strFx);
		if (it != m_Effects.end())
		{
			if (pLoader->Is_Current(it->second))
				it->second.p->Start_FadeOut(0.4f);
			it->second.Clear();
		}
	}
	return true;
}

void CMonster::Start_LaunchSmokeFx()
{
	if (!m_bActive || nullptr == m_pTransformCom)
		return;

	auto pLoader = CEffect_Loader::GetInstance();
	FX_HANDLE& hSlot = m_Effects[L"LaunchSmoke"];
	if (pLoader->Is_Current(hSlot))
		return;

	_float3 vFaceCam{};
	XMStoreFloat3(&vFaceCam, 
		XMVectorNegate(XMLoadFloat4(m_pGameInstance_Proxy->Get_CamLook())));

	hSlot.Clear();
	pLoader->Spawn(L"LaunchSmoke", Get_LevelIndex(),
		_float3(0.f, 0.f, 0.f),
		vFaceCam,
		_float3(0.f, 0.f, 0.f),
		m_pTransformCom->Get_WorldMatrixPtr(), nullptr, &hSlot);
}

void CMonster::Stop_LaunchSmokeFx(_bool bImmediate)
{
	auto it = m_Effects.find(L"LaunchSmoke");
	if (it == m_Effects.end())
		return;

	auto pLoader = CEffect_Loader::GetInstance();
	FX_HANDLE& hSlot = it->second;
	if (pLoader->Is_Current(hSlot))
	{
		if (bImmediate)
			hSlot.p->EffectContainer_Stop();
		else
			hSlot.p->EffectContainer_StopAfterEmission();
	}

	hSlot.Clear();
}

void CMonster::Stop_AllFx(_bool bImmediate)
{
	auto pLoader = CEffect_Loader::GetInstance();

	for (auto& Pair : m_Effects)
	{
		FX_HANDLE& h = Pair.second;
		if (pLoader->Is_Current(h))
		{
			if (bImmediate)
				h.p->EffectContainer_Stop();
			else
				h.p->Start_FadeOut(0.4f);
		}
		h.Clear();
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

void CMonster::Compute_SpatPivot()
{
	auto it = m_PartObjects.find(L"Body");
	if (it == m_PartObjects.end())
		return;

	CMonsterPart* pBody = dynamic_cast<CMonsterPart*>(it->second);
	if (nullptr == pBody)
		return;

	CModel* pModel = pBody->Get_Model();
	if (nullptr == pModel)
		return;

	_float3 vMin, vMax;
	pModel->Get_ModelAABB(&vMin, &vMax);

	m_vSpatPivot = _float3(
		(vMin.x + vMax.x) * 0.5f,
		(vMin.y + vMax.y) * 0.5f,
		(vMin.z + vMax.z) * 0.5f);
}

void CMonster::Open_BodyCheckBlock()
{
	if (m_fBodyCheckOffTime <= 0.f)
		m_bHadBodyCheckTrait = Has_Trait(MT_BODYCHECK_DAMAGE);

	m_TraitFlags &= ~MT_BODYCHECK_DAMAGE;
	m_fBodyCheckOffTime = s_fBodyCheckOffDuration;
}

void CMonster::Update_SpatPivot_FromBone()
{
	auto it = m_PartObjects.find(L"Body");
	if (it == m_PartObjects.end())
		return;

	CMonsterPart* pBody =
		dynamic_cast<CMonsterPart*>(it->second);
	if (nullptr == pBody)
		return;

	const _float4x4* pBone =
		pBody->Get_BoneMatrixPtr("CenterL");
	if (nullptr == pBone)
		pBone = pBody->Get_BoneMatrixPtr("RotL");

	if (pBone)
		m_vSpatPivot = _float3(
			pBone->_41, pBone->_42, pBone->_43);
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
	Stop_AllFx(true);
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);
	m_pCaptor = nullptr;
	Set_Active(false);
}

void CMonster::On_SpatBegin()
{
	m_pCaptor = nullptr;
	Set_Active(true);
	
	Enable_Controller(false);
	Enable_Colliders(false);

	Update_SpatPivot_FromBone();

	Change_State(MONSTER_STATE_TYPE::SPAT);
}

void CMonster::On_SpatEnd()
{
	Despawn();
}

void CMonster::Despawn()
{
	Stop_AllFx(false);
	Play_DeathFX();

	Enable_Colliders(false);
	Enable_Controller(false);
	Set_Active(false);
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
