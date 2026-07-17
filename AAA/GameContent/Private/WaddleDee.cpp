#include "WaddleDee.h"
#include "WaddleDee_Body.h"
#include "LevelDesign_LoadTypes.h"
#include "GameContrnt_Events.h"
#include "GameContent_const.h"

#include "GameInstance.h"
#include "Parsing_Utils.h"
#include "Math_Utils.h"

namespace
{
	constexpr const _char* s_szIdleClip = "Wait";
	constexpr const _char* s_szWalkClip = "Walk";
	constexpr const _char* s_szGreetClip = "WaveHand";
	constexpr const _char* s_szHitClip = "Surprise";
	constexpr _ubyte s_byInteractKey = DIK_F;

	constexpr _float s_fWalkTimeLimit = 6.f;
	constexpr _float s_fArriveDistance = 0.15f;
	constexpr _float s_fWanderDelayMin = 2.f;
	constexpr _float s_fWanderDelayMax = 5.f;
	constexpr _float s_fGreetCooldown = 1.5f;

	constexpr _float s_fHurtBoxRadius = 0.6f;
	constexpr _float s_fHurtBoxHeight = 0.25f;

	static_assert(s_fHurtBoxRadius > 0.f);
	static_assert(s_fHurtBoxHeight >= 0.f);
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
{
}

HRESULT CWaddleDee::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (nullptr != pArg)
	{
		const LD_OBJECT_DESC* pDesc = static_cast<const LD_OBJECT_DESC*>(pArg);
		if (!pDesc->strAIVariation.empty())
			m_strFixedAnim = pDesc->strAIVariation;
	}

	if (FAILED(Ready_PartObjects()))
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
			Update_Greet();
		break;

	case WADDLEDEE_STATE::HIT:
		if (!bEditMode)
			Update_Hit();
		break;
	}

	if (!bEditMode && WADDLEDEE_STATE::GREET != m_eState && WADDLEDEE_STATE::HIT != m_eState)
		Check_Interact();
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

void CWaddleDee::Damaged(const ATTACK_INFO& tInfo)
{
	if (!m_bActive || m_pGameInstance_Proxy->Is_EditMode())
		return;

	const _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	const _vector vAttackerPosition = XMLoadFloat3(&tInfo.vAttackerPos);
	const _vector vToAttacker = XMVectorSetY(vAttackerPosition - vPosition, 0.f);
	const _float fDistanceSq = XMVectorGetX(XMVector3LengthSq(vToAttacker));

	if (fDistanceSq > FLT_EPSILON)
		m_pTransformCom->LookAt(vPosition + vToAttacker);

	Change_State(WADDLEDEE_STATE::HIT);
}

void CWaddleDee::Set_Active(_bool bActive)
{
	__super::Set_Active(bActive);

	if (nullptr != m_pHurtBox)
		m_pHurtBox->Set_Enabled(bActive);
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
	m_strAppliedFixedAnim.clear();

	Change_State(WADDLEDEE_STATE::IDLE);
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

	if (!m_pBody->Has_Animation(s_szIdleClip)
		|| !m_pBody->Has_Animation(s_szWalkClip)
		|| !m_pBody->Has_Animation(s_szGreetClip)
		|| !m_pBody->Has_Animation(s_szHitClip))
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
		m_pBody->Get_Animator()->Play(s_szWalkClip, true, true, 0.f);
		break;

	case WADDLEDEE_STATE::GREET:
		m_pBody->Get_Animator()->Play(s_szGreetClip, false, true);
		break;

	case WADDLEDEE_STATE::HIT:
		m_pBody->Get_Animator()->Play(s_szHitClip, false, true);
		break;
	}
}

void CWaddleDee::Check_Interact()
{
	if (m_fGreetCooldown > 0.f || !Find_Player())
		return;

	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vPlayerPosition = m_pPlayer->Get_Transform()->Get_State(STATE::POSITION);
	_vector vToPlayer = XMVectorSetY(vPlayerPosition - vPosition, 0.f);
	const _float fDistanceSq = XMVectorGetX(XMVector3LengthSq(vToPlayer));

	if (fDistanceSq > m_fInteractRadius * m_fInteractRadius)
		return;

	if (!m_pGameInstance_Proxy->Key_Down(s_byInteractKey))
		return;

	if (fDistanceSq > FLT_EPSILON)
		m_pTransformCom->LookAt(vPosition + vToPlayer);

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

	if (!m_bWander || !m_strFixedAnim.empty())
		return;

	m_fStateTimer -= fTimeDelta;

	if (m_fStateTimer <= 0.f)
		Change_State(WADDLEDEE_STATE::WALK);
}


void CWaddleDee::Play_Idle()
{
	const _string strClip = m_strFixedAnim.empty() ? s_szIdleClip : WstrToStr(m_strFixedAnim);

	m_pBody->Get_Animator()->Play(strClip, true, true, 0.f);
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

	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTarget = XMLoadFloat3(&m_vWalkTarget);
	_vector vToTarget = XMVectorSetY(vTarget - vPosition, 0.f);
	const _float fDistanceSq = XMVectorGetX(XMVector3LengthSq(vToTarget));

	if (fDistanceSq <= s_fArriveDistance * s_fArriveDistance || m_fStateTimer <= 0.f)
	{
		Change_State(WADDLEDEE_STATE::IDLE);
		return;
	}

	m_pTransformCom->LookAt_Smooth(vTarget, fTimeDelta);

	const _float fDistance = sqrtf(fDistanceSq);
	const _float fMoveDistance = min(m_fWalkSpeed * fTimeDelta, fDistance);
	vPosition += XMVectorScale(vToTarget, fMoveDistance / fDistance);
	vPosition = XMVectorSetY(vPosition, m_vBasePos.y);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vPosition, 1.f));

	if (fMoveDistance >= fDistance)
		Change_State(WADDLEDEE_STATE::IDLE);
}

void CWaddleDee::Update_Greet()
{
	if (!m_pBody->Get_Animator()->Is_Finished())
		return;

	m_fGreetCooldown = s_fGreetCooldown;
	Change_State(WADDLEDEE_STATE::IDLE);
}

void CWaddleDee::Update_Hit()
{
	if (!m_pBody->Get_Animator()->Is_Finished())
		return;

	Change_State(WADDLEDEE_STATE::IDLE);
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