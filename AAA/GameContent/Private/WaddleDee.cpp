#include "WaddleDee.h"
#include "WaddleDee_Body.h"
#include "WaddleDee_Hat.h"
#include "LevelDesign_LoadTypes.h"
#include "GameContent_Events.h"
#include "GameContent_const.h"

#include "GameInstance.h"
#include "Parsing_Utils.h"
#include "Math_Utils.h"

namespace
{
	struct WADDLEDEE_VARIATION_ANIM_DESC
	{
		const _tchar* szVariation;
		const _tchar* szAnimation;
	};

	constexpr WADDLEDEE_VARIATION_ANIM_DESC s_WaddleDeeVariationAnimDescs[] =
	{
			{ L"BreakWait", L"Break" },
			{ L"DrinkCanJuiceWait", L"DrinkCanJuiceWaitA" },
			{ L"DrinkCupJuiceWait", L"DrinkCupJuiceWaitA" },
			{ L"FishingWait", L"FishingWaitVillager" },
			{ L"LookWait", L"TownLookAround" },
			{ L"MoveOnRail", L"Wait" },
			{ L"ReadBookWait", L"ReadBookA" },
			{ L"SitTalkALegUpWait", L"SitTalkALegUp" },
			{ L"SitTalkAWait", L"SitTalkA" },
			{ L"SitTalkBLegUpWait", L"SitTalkBLegUp" },
			{ L"SitTalkBWait", L"SitTalkB" },
			{ L"SitTalkListenWait", L"SitTalkListen" },
			{ L"SitWait", L"SitWait" },
			{ L"SleepSit", L"SleepSit" },
			{ L"StreetLiveAudienceWait", L"TownWait" },
			{ L"Talk1Wait", L"Talk1" },
			{ L"Talk2Wait", L"Talk2" },
			{ L"Talk3AWait", L"Talk3A" },
			{ L"Talk3BWait", L"Talk3B" },
			{ L"Talk3ListenWait", L"Talk3Listen" },
			{ L"WorryWait", L"ShopSadWait" },
			{ L"WaitA", L"VassalWaitA" },
			{ L"WaitB", L"VassalWaitB" },
	};

	struct WADDLEDEE_NPC_DESC
	{
		const _tchar* szObjectName;
		const _tchar* szHatModelProtoTag;
		const _char* szSocketBone;
		const _tchar* szDefaultFixedAnim;
	};

	constexpr WADDLEDEE_NPC_DESC s_WaddleDeeNpcDescs[] =
	{
			{ L"PharmacyClerk",        L"Prototype_Component_Model_TownHat_Pharmacy",        "HatL", nullptr },
			{ L"FoodShopClerk",        L"Prototype_Component_Model_TownHat_FoodShop",        "HatL", nullptr },
			{ L"MgameFoodClerk",       L"Prototype_Component_Model_TownHat_FoodShop",        "HatL", nullptr },
			{ L"MgameBallClerk",       L"Prototype_Component_Model_TownHat_RollingBall",     "HatL", nullptr },
			{ L"DeliveryServiceClerk", L"Prototype_Component_Model_TownHat_DeliveryService", "HatL", nullptr },
			{ L"ArenaClerk",           L"Prototype_Component_Model_TownHat_Arena",           "HatL", nullptr },
			{ L"KnowledgeWaddleDee",   L"Prototype_Component_Model_TownHat_Knowledge",       "HatL", L"KnowledgeResearch" }
	};

	const WADDLEDEE_NPC_DESC* Find_NpcDesc(const _wstring& strObjectName)
	{
		for (const WADDLEDEE_NPC_DESC& Desc : s_WaddleDeeNpcDescs)
		{
			if (JsonUtils::Equals_NoCase(strObjectName.c_str(), Desc.szObjectName))
				return &Desc;
		}

		return nullptr;
	}

	constexpr _float s_fAnimationSpeed = 1.5f;
	constexpr _float s_fBlendDuration = 0.2f;
	constexpr const _char* s_strIdleClip = "Wait";
	constexpr const _char* s_strWalkClip = "Walk";
	constexpr const _char* s_strAngryClip = "ImpatienceOneTime";

	struct INTERACT_ANIM_DESC
	{
		_ubyte byKey;
		const _char* szClip;
	};

	constexpr INTERACT_ANIM_DESC s_InteractAnimDescs[] = {
		  { DIK_F, "WaveHand" },
		  { DIK_G, "Yay" }
	};

	struct HIT_ANIM_SET
	{
		const _char* szStart;
		const _char* szWait;
		const _char* szEnd;
	};

	constexpr HIT_ANIM_SET s_NormalHitAnimSets[] =
	{
		  { "HitDown1Start", "HitDown1Wait", "HitDown1End" },
		  { "HitDown2Start", "HitDown2Wait", "HitDown2End" }
	};

	constexpr HIT_ANIM_SET s_SitHitAnimSet =
	{ "SitHitDownStart", "SitHitDownWait", "SitHitDownEnd" };

	constexpr const _tchar* s_SittingFixedAnims[] =
	{
		L"SitWait",
		L"SitTableWait",
		L"SleepSit",
		L"SitTalkA",
		L"SitTalkALegUp",
		L"SitTalkB",
		L"SitTalkBLegUp",
		L"SitTalkListen"
	};

	constexpr _uint s_iNormalHitAnimSetCount = static_cast<_uint>(sizeof(s_NormalHitAnimSets) / sizeof(s_NormalHitAnimSets[0]));
	
	constexpr const _tchar* s_ArenaBattleAnimClips[] =
	{
			L"Cheering1",
			L"Cheering2",
			L"Cheering3",
			L"Cheering4"
	};

	constexpr _uint s_iArenaBattleAnimClipCount = static_cast<_uint>(sizeof(s_ArenaBattleAnimClips) / sizeof(s_ArenaBattleAnimClips[0]));

	constexpr _float s_fRotationPerSec = 180.f;
	constexpr _float s_fWalkTimeLimit = 6.f;
	constexpr _float s_fArriveDistance = 0.15f;
	constexpr _float s_fWanderDelayMin = 2.f;
	constexpr _float s_fWanderDelayMax = 5.f;
	constexpr _float s_fGreetCooldown = 1.5f;

	constexpr _float s_fHurtBoxRadius = 0.6f;
	constexpr _float s_fHurtBoxHeight = 0.25f;

	constexpr _float s_fCCT_Radius = 0.4f;
	constexpr _float s_fCCT_Height = 0.25f;

	static_assert(s_fBlendDuration > 0.f);
	static_assert(s_fAnimationSpeed > 0.f);
	static_assert(s_fRotationPerSec > 0.f);
	static_assert(s_fCCT_Radius > 0.f);
	static_assert(s_fCCT_Height >= 0.f);
	static_assert(s_fHurtBoxRadius > 0.f);
	static_assert(s_fHurtBoxHeight >= 0.f);
	static_assert(s_iNormalHitAnimSetCount > 0);
	static_assert(s_iArenaBattleAnimClipCount > 0);
}

CWaddleDee::CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice, pContext }
	, m_strFixedAnim{ L"" }
	, m_fInteractRadius{ 2.f }
	, m_bWander{ false }
	, m_fWanderRadius{ 3.f }
	, m_fWalkSpeed{ 1.5f }
{
}

CWaddleDee::CWaddleDee(const CWaddleDee& Prototype)
	: CCharacter(Prototype)
	, m_strFixedAnim(Prototype.m_strFixedAnim)
	, m_fInteractRadius(Prototype.m_fInteractRadius)
	, m_bWander(Prototype.m_bWander)
	, m_fWanderRadius(Prototype.m_fWanderRadius)
	, m_fWalkSpeed(Prototype.m_fWalkSpeed)
	, m_strObjectName(Prototype.m_strObjectName)
	, m_bUseMovement(Prototype.m_bUseMovement)
{
}

HRESULT CWaddleDee::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_RotationPerSec(s_fRotationPerSec);

	if (nullptr != pArg)
	{
		const LD_OBJECT_DESC* pDesc = static_cast<const LD_OBJECT_DESC*>(pArg);
		m_strObjectName = pDesc->strObjectName;
		m_bUseMovement = !JsonUtils::Equals_NoCase(m_strObjectName.c_str(), L"ArenaSpectator");

		if (!pDesc->strAIVariation.empty())
			m_strFixedAnim = pDesc->strAIVariation;

		const WADDLEDEE_NPC_DESC* pNpcDesc = Find_NpcDesc(m_strObjectName);
		if (m_strFixedAnim.empty() && nullptr != pNpcDesc && nullptr != pNpcDesc->szDefaultFixedAnim)
			m_strFixedAnim = pNpcDesc->szDefaultFixedAnim;
	}

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	if (FAILED(Ready_Movement()))
		return E_FAIL;

	if (FAILED(Ready_AnimEvents()))
		return E_FAIL;

	if (FAILED(Ready_HurtBox()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	Change_State(WADDLEDEE_STATE::IDLE);

	return S_OK;
}

void CWaddleDee::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Update(fTimeDelta);

	const _bool bEditMode = m_pGameInstance_Proxy->Is_EditMode();

	if (bEditMode && m_bUseMovement)
		m_pMovement->Sync_To_Controller();

	if (!bEditMode && !m_bBasePosCaptured)
	{
		XMStoreFloat3(&m_vBasePos, m_pTransformCom->Get_State(STATE::POSITION));
		m_bBasePosCaptured = true;
	}

	if (!bEditMode && m_fGreetCooldown > 0.f)
		m_fGreetCooldown = max(0.f, m_fGreetCooldown - fTimeDelta);

	switch (m_eState)
	{
	case WADDLEDEE_STATE::IDLE:
		Update_Idle(bEditMode ? 0.f : fTimeDelta);
		break;

	case WADDLEDEE_STATE::WALK:
		if (!bEditMode)
			Update_Walk(fTimeDelta);
		break;

	case WADDLEDEE_STATE::GREET:
		if (!bEditMode)
			Update_Greet(fTimeDelta);
		break;

	case WADDLEDEE_STATE::HIT:
		if (!bEditMode)
			Update_Hit(fTimeDelta);
		break;

	case WADDLEDEE_STATE::ANGRY:
		if (!bEditMode)
			Update_Angry(fTimeDelta);
		break;
	}

#ifdef _DEBUG
	if (!bEditMode
		&& WADDLEDEE_STATE::GREET != m_eState
		&& WADDLEDEE_STATE::HIT != m_eState
		&& WADDLEDEE_STATE::ANGRY != m_eState)
		Check_Interact();
#endif
}

void CWaddleDee::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);

	if (m_pHurtBox->Is_Enabled())
	{
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
	}
}

void CWaddleDee::Damaged(const ATTACK_INFO&)
{
	if (!m_bActive || m_pGameInstance_Proxy->Is_EditMode())
		return;

	if (WADDLEDEE_STATE::GREET == m_eState
		|| WADDLEDEE_STATE::HIT == m_eState
		|| WADDLEDEE_STATE::ANGRY == m_eState)
		return;

	XMStoreFloat3(&m_vLookOrigin, XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f)));

	Change_State(WADDLEDEE_STATE::HIT);
}

void CWaddleDee::Set_Active(_bool bActive)
{
	__super::Set_Active(bActive);

	m_pHurtBox->Set_Enabled(bActive);

	if (m_bUseMovement)
		m_pController->Set_Enabled(bActive);
}

void CWaddleDee::On_Deserialized()
{
	__super::On_Deserialized();

	if (FAILED(Validate_Initialized()))
	{
#ifdef _DEBUG
		OutputDebugStringW(L"[CWaddleDee] Invalid serialized properties.\n");
#endif
		Set_Active(false);
		return;
	}

	m_fStateTimer = 0.f;
	m_fGreetCooldown = 0.f;

	m_bBasePosCaptured = false;
	m_vBasePos = {};
	m_vWalkTarget = {};

	m_pPlayer = nullptr;
	m_strInteractClip.clear();
	m_strAppliedFixedAnim.clear();

	Change_State(WADDLEDEE_STATE::IDLE);

	if (m_bUseMovement)
	{
		Sync_MovementStats();
		m_pMovement->Sync_To_Controller();
	}
}

HRESULT CWaddleDee::Ready_PartObjects()
{
	CWaddleDee_Body::WADDLEDEE_BODY_DESC BodyDesc{};
	BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

	if (FAILED(Add_PartObject(m_iPrototypeLevel, CWaddleDee_Body::PROTOTYPE_TAG, CWaddleDee_Body::PART_TAG, &BodyDesc)))
		return E_FAIL;

	auto iter = m_PartObjects.find(CWaddleDee_Body::PART_TAG);
	if (iter == m_PartObjects.end())
		return E_FAIL;

	m_pBody = dynamic_cast<CWaddleDee_Body*>(iter->second);
	if (nullptr == m_pBody)
		return E_FAIL;

	const WADDLEDEE_NPC_DESC* pNpcDesc = Find_NpcDesc(m_strObjectName);
	if (nullptr == pNpcDesc || nullptr == pNpcDesc->szHatModelProtoTag || nullptr == pNpcDesc->szSocketBone)
		return S_OK;

	const _float4x4* pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr(pNpcDesc->szSocketBone);
	if (nullptr == pSocketBoneMatrix)
		return S_OK;

	CWaddleDee_Hat::WADDLEDEE_HAT_DESC HatDesc{};
	HatDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	HatDesc.pSocketBoneMatrix = pSocketBoneMatrix;
	HatDesc.szModelProtoTag = pNpcDesc->szHatModelProtoTag;

	if (SUCCEEDED(Add_PartObject(m_iPrototypeLevel, CWaddleDee_Hat::PROTOTYPE_TAG, CWaddleDee_Hat::PART_TAG, &HatDesc)))
	{
		auto HatIter = m_PartObjects.find(CWaddleDee_Hat::PART_TAG);
		if (HatIter != m_PartObjects.end())
			m_pHat = dynamic_cast<CWaddleDee_Hat*>(HatIter->second);
	}

	return S_OK;
}

HRESULT CWaddleDee::Ready_Movement()
{
	if (!m_bUseMovement)
		return S_OK;

	_float3 vFootPos{};
	XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));

	m_pController = Add_Component<CController>(TEXT("Com_Controller"), CController::Create(m_pDevice, m_pContext));
	if (nullptr == m_pController)
		return E_FAIL;

	CController::CONTROLLER_DESC ControllerDesc{};
	ControllerDesc.vFootPos = vFootPos;
	ControllerDesc.fRadius = s_fCCT_Radius;
	ControllerDesc.fHeight = s_fCCT_Height;
	ControllerDesc.pOwner = this;

	if (FAILED(m_pController->Initialize(&ControllerDesc)))
		return E_FAIL;

	m_pMovement = Add_Component<CMovement>(TEXT("Com_Movement"), CMovement::Create(m_pDevice, m_pContext));
	if (nullptr == m_pMovement)
		return E_FAIL;

	m_pMovement->Set_Refs(m_pTransformCom, m_pController->Get_Raw());
	Sync_MovementStats();

	return S_OK;
}

HRESULT CWaddleDee::Ready_AnimEvents()
{
	if (nullptr == m_pBody)
		return E_FAIL;

	CAnimator* pAnim = m_pBody->Get_Animator();
	if (nullptr == pAnim)
		return E_FAIL;

	pAnim->Set_EventCallback(
		[this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
		{
			switch (static_cast<EANIM_EVENT>(e.iEventType))
			{
			case EANIM_EVENT::SetEye:
				if (phase == ANIM_EVENT_PHASE::POINT)
					m_pBody->Set_Eye(static_cast<_uint>(e.iIntParam));
				break;

			default:
				break;
			}
		});

	return S_OK;
}

HRESULT CWaddleDee::Ready_HurtBox()
{
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.f, 0.f };
	ColliderDesc.fRadius = s_fHurtBoxRadius;
	ColliderDesc.fHeight = s_fHurtBoxHeight;

	m_pHurtBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag, TEXT("Com_HurtBox"), &ColliderDesc);
	if (nullptr == m_pHurtBox)
		return E_FAIL;

	m_pHurtBox->Set_OnEnter([this](CCollider* pOther)
		{
			const _uint iGroup = pOther->Get_RegisteredGroup();
			CGameObject* pAttacker = pOther->Get_Owner();

			const _bool bBombHit = ETOUI(COLLISION_LAYER::PLAYER_BOMB) == iGroup;
			const _bool bDirectPlayerHit = ETOUI(COLLISION_LAYER::PLAYER_HIT) == iGroup && Find_Player() && pAttacker == m_pPlayer;
			if (!bBombHit && !bDirectPlayerHit)
				return;

			ATTACK_INFO AttackInfo{};
			AttackInfo.pAttacker = pAttacker;
			AttackInfo.eHitType = bBombHit ? HIT_TYPE::BOMB : HIT_TYPE::SLIDE;
			XMStoreFloat3(&AttackInfo.vAttackerPos, pAttacker->Get_Transform()->Get_State(STATE::POSITION));

			Damaged(AttackInfo);
		});

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::ENV_HURT));

	return S_OK;
}

HRESULT CWaddleDee::Validate_Initialized()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;

	if (nullptr == m_pBody || nullptr == m_pBody->Get_Animator() || nullptr == m_pHurtBox)
		return E_FAIL;

	if (!m_pBody->Has_Animation(s_strIdleClip)
		|| !m_pBody->Has_Animation(s_strWalkClip)
		|| !m_pBody->Has_Animation(s_strAngryClip))
		return E_FAIL;

	for (const INTERACT_ANIM_DESC& InteractAnimDesc : s_InteractAnimDescs)
	{
		if (!m_pBody->Has_Animation(InteractAnimDesc.szClip))
			return E_FAIL;
	}

	for (const HIT_ANIM_SET& HitAnimSet : s_NormalHitAnimSets)
	{
		if (!m_pBody->Has_Animation(HitAnimSet.szStart)
			|| !m_pBody->Has_Animation(HitAnimSet.szWait)
			|| !m_pBody->Has_Animation(HitAnimSet.szEnd))
			return E_FAIL;
	}

	if (!m_pBody->Has_Animation(s_SitHitAnimSet.szStart)
		|| !m_pBody->Has_Animation(s_SitHitAnimSet.szWait)
		|| !m_pBody->Has_Animation(s_SitHitAnimSet.szEnd))
		return E_FAIL;

	if (!m_strFixedAnim.empty())
	{
		const _string strFixedAnim = WstrToStr(m_strFixedAnim);
		if (!m_pBody->Has_Animation(strFixedAnim.c_str()))
			return E_FAIL;
	}

	if (!MathUtils::Is_FiniteFloat(m_fInteractRadius)
		|| !MathUtils::Is_FiniteFloat(m_fWanderRadius)
		|| !MathUtils::Is_FiniteFloat(m_fWalkSpeed))
		return E_FAIL;

	if (m_fInteractRadius <= 0.f || m_fWanderRadius <= 0.f || m_fWalkSpeed <= 0.f)
		return E_FAIL;

	return S_OK;
}

void CWaddleDee::Sync_MovementStats()
{
	m_pMovement->Set_MoveSpeed(m_fWalkSpeed);
	m_pMovement->Set_RotSpeed(s_fRotationPerSec);
}

_bool CWaddleDee::Is_Sitting() const
{
	for (const _tchar* pFixedAnim : s_SittingFixedAnims)
	{
		if (m_strFixedAnim == pFixedAnim)
			return true;
	}

	return false;
}

void CWaddleDee::Change_State(WADDLEDEE_STATE eState)
{
	m_eState = eState;
	m_fStateTimer = 0.f;

	switch (m_eState)
	{
	case WADDLEDEE_STATE::IDLE:
		m_fStateTimer = m_pGameInstance_Proxy->RandomFloat(s_fWanderDelayMin, s_fWanderDelayMax);
		Play_Idle();
		break;

	case WADDLEDEE_STATE::WALK:
		m_fStateTimer = s_fWalkTimeLimit;
		Pick_WalkTarget();
		m_pBody->Get_Animator()->Play(s_strWalkClip, true, true, 0.f, s_fAnimationSpeed);
		break;

	case WADDLEDEE_STATE::GREET:
		m_pBody->Get_Animator()->Play(m_strInteractClip, false, true, s_fBlendDuration, s_fAnimationSpeed);
		break;

	case WADDLEDEE_STATE::HIT:
	{
		const HIT_ANIM_SET* pHitAnimSet = &s_SitHitAnimSet;

		if (!Is_Sitting())
		{
			const _uint iAnimSetIndex = static_cast<_uint>(m_pGameInstance_Proxy->RandomInt(0, static_cast<_int>(s_iNormalHitAnimSetCount) - 1));
			pHitAnimSet = &s_NormalHitAnimSets[iAnimSetIndex];
		}

		CAnimator::ANI_PLAY_INFO AnimInfo{};
		AnimInfo.bLoop = false;
		AnimInfo.bRestart = true;
		AnimInfo.fSpeed = s_fAnimationSpeed;

		AnimInfo.strAniName = pHitAnimSet->szStart;
		m_pBody->Get_Animator()->Play(&AnimInfo);

		AnimInfo.strAniName = pHitAnimSet->szWait;
		m_pBody->Get_Animator()->Enqueue(AnimInfo);

		AnimInfo.strAniName = pHitAnimSet->szEnd;
		m_pBody->Get_Animator()->Enqueue(AnimInfo);
		break;
	}

	case WADDLEDEE_STATE::ANGRY:
	{
		const _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vFixedLook = XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f);

		if (Find_Player())
		{
			const _vector vPlayerPosition = m_pPlayer->Get_Transform()->Get_State(STATE::POSITION);
			const _vector vToPlayer = XMVectorSetY(vPlayerPosition - vPosition, 0.f);

			if (XMVectorGetX(XMVector3LengthSq(vToPlayer)) > FLT_EPSILON)
				vFixedLook = vToPlayer;
		}

		XMStoreFloat3(&m_vLookTarget, XMVector3Normalize(vFixedLook));
		m_pBody->Get_Animator()->Play(s_strAngryClip, false, true, s_fBlendDuration, s_fAnimationSpeed);
		break;
	}
	}
}

void CWaddleDee::Check_Interact()
{
	if (m_fGreetCooldown > 0.f)
		return;

	const _char* pInteractClip = nullptr;

	for (const INTERACT_ANIM_DESC& InteractAnimDesc : s_InteractAnimDescs)
	{
		if (!m_pGameInstance_Proxy->Key_Down(InteractAnimDesc.byKey))
			continue;

		pInteractClip = InteractAnimDesc.szClip;
		break;
	}

	if (nullptr == pInteractClip || !Find_Player())
		return;

	const _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	const _vector vPlayerPosition = m_pPlayer->Get_Transform()->Get_State(STATE::POSITION);
	const _vector vToPlayer = XMVectorSetY(vPlayerPosition - vPosition, 0.f);
	const _float fDistanceSq = XMVectorGetX(XMVector3LengthSq(vToPlayer));

	if (fDistanceSq > m_fInteractRadius * m_fInteractRadius)
		return;

	const _vector vCurrentLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
	_vector vFixedLook = vCurrentLook;

	if (fDistanceSq > FLT_EPSILON)
		vFixedLook = XMVector3Normalize(vToPlayer);

	XMStoreFloat3(&m_vLookOrigin, vCurrentLook);
	XMStoreFloat3(&m_vLookTarget, vFixedLook);

	m_strInteractClip = pInteractClip;
	Change_State(WADDLEDEE_STATE::GREET);
}

_bool CWaddleDee::Find_Player()
{
	if (nullptr != m_pPlayer)
		return true;

	PLAYER_QUERY PlayerQuery{};
	m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &PlayerQuery);
	m_pPlayer = PlayerQuery.pPlayer;

	return nullptr != m_pPlayer;
}

void CWaddleDee::Update_Idle(_float fTimeDelta)
{
	if (m_strAppliedFixedAnim != m_strFixedAnim)
	{
		Change_State(WADDLEDEE_STATE::IDLE);
		return;
	}

	if (!m_bUseMovement || !m_bWander || !m_strFixedAnim.empty())
		return;

	m_fStateTimer -= fTimeDelta;

	if (m_fStateTimer <= 0.f)
		Change_State(WADDLEDEE_STATE::WALK);
}

void CWaddleDee::Play_Idle()
{
	const _string strClip = m_strFixedAnim.empty() ? s_strIdleClip : WstrToStr(m_strFixedAnim);

	m_pBody->Get_Animator()->Play(strClip, true, true, s_fBlendDuration, s_fAnimationSpeed);
	m_strAppliedFixedAnim = m_strFixedAnim;
}

void CWaddleDee::Pick_WalkTarget()
{
	const _float fAngle = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);
	const _float fDistance = m_pGameInstance_Proxy->RandomFloat(m_fWanderRadius * 0.3f, m_fWanderRadius);

	_float fSin = 0.f;
	_float fCos = 0.f;
	XMScalarSinCos(&fSin, &fCos, fAngle);

	m_vWalkTarget = {
			m_vBasePos.x + fCos * fDistance,
			m_vBasePos.y,
			m_vBasePos.z + fSin * fDistance
	};
}

void CWaddleDee::Update_Walk(_float fTimeDelta)
{
	if (!m_bWander || !m_strFixedAnim.empty())
	{
		Change_State(WADDLEDEE_STATE::IDLE);
		return;
	}

	m_fStateTimer -= fTimeDelta;

	const _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	const _vector vTarget = XMLoadFloat3(&m_vWalkTarget);
	const _vector vToTarget = XMVectorSetY(vTarget - vPosition, 0.f);
	const _float fDistanceSq = XMVectorGetX(XMVector3LengthSq(vToTarget));

	if (fDistanceSq <= s_fArriveDistance * s_fArriveDistance || m_fStateTimer <= 0.f)
	{
		Change_State(WADDLEDEE_STATE::IDLE);
		return;
	}

	m_pMovement->Move(vToTarget, fTimeDelta);
}

void CWaddleDee::Update_Greet(_float fTimeDelta)
{
	const _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);

	if (!m_pBody->Get_Animator()->Is_Finished())
	{
		const _vector vFixedTarget = vPosition + XMLoadFloat3(&m_vLookTarget);
		m_pTransformCom->LookAt_Smooth(vFixedTarget, fTimeDelta);
		return;
	}

	const _vector vOriginalTarget = vPosition + XMLoadFloat3(&m_vLookOrigin);

	if (!m_pTransformCom->LookAt_Smooth(vOriginalTarget, fTimeDelta))
		return;

	m_fGreetCooldown = s_fGreetCooldown;
	Change_State(WADDLEDEE_STATE::IDLE);
}

void CWaddleDee::Update_Hit(_float fTimeDelta)
{
	if (!m_pBody->Get_Animator()->Is_Finished())
		return;

	Change_State(WADDLEDEE_STATE::ANGRY);
}

void CWaddleDee::Update_Angry(_float fTimeDelta)
{
	const _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);

	if (!m_pBody->Get_Animator()->Is_Finished())
	{
		const _vector vFixedTarget = vPosition + XMLoadFloat3(&m_vLookTarget);
		m_pTransformCom->LookAt_Smooth(vFixedTarget, fTimeDelta);
		return;
	}

	const _vector vOriginalTarget = vPosition + XMLoadFloat3(&m_vLookOrigin);

	if (!m_pTransformCom->LookAt_Smooth(vOriginalTarget, fTimeDelta))
		return;

	Change_State(WADDLEDEE_STATE::IDLE);
}

_wstring CWaddleDee::Resolve_FixedAnim(const _wstring& strVariation, _uint iSeed)
{
	if (JsonUtils::Equals_NoCase(strVariation.c_str(), L"Battle"))
		return s_ArenaBattleAnimClips[iSeed % s_iArenaBattleAnimClipCount];

	for (const WADDLEDEE_VARIATION_ANIM_DESC& Desc : s_WaddleDeeVariationAnimDescs)
	{
		if (JsonUtils::Equals_NoCase(strVariation.c_str(), Desc.szVariation))
			return Desc.szAnimation;
	}

	return {};
}

CWaddleDee* CWaddleDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWaddleDee* pInstance = new CWaddleDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWaddleDee");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWaddleDee::Clone(void* pArg)
{
	CWaddleDee* pInstance = new CWaddleDee(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWaddleDee");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWaddleDee::Free()
{
	__super::Free();
}