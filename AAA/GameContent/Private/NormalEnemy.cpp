#include "NormalEnemy.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"
#include "Monster_StateMachine.h"
#include "NormalEnemy_Brain.h"

// Parts
#include "NormalEnemy_Body.h"

// 공용 상태
#include "Monster_State_Idle.h"
#include "Monster_State_Detect.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_Chase.h"
#include "Monster_State_Captured.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Spat.h"

// 전용 상태

CNormalEnemy::CNormalEnemy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster { pDevice, pContext }
{
}

CNormalEnemy::CNormalEnemy(const CNormalEnemy& Prototype)
	: CMonster( Prototype )
{
}

HRESULT CNormalEnemy::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CNormalEnemy::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

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

void CNormalEnemy::Priority_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CNormalEnemy::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Update(fTimeDelta);
}

void CNormalEnemy::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);
}

_bool CNormalEnemy::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
	Out.fRadius = { 0.6f };
	Out.fHeight = { 0.75f };
	return true;
}

CAnimator* CNormalEnemy::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CNormalEnemy::Create_Brain()
{
	return CNormalEnemy_Brain::Create(this);
}

HRESULT CNormalEnemy::Ready_State(CMonster_StateMachine* pStateMachine)
{
	return S_OK;
}

HRESULT CNormalEnemy::Ready_AnimEvents()
{
	return S_OK;
}

HRESULT	CNormalEnemy::Ready_PartObjects()
{
	// Body 
	m_pBody = Add_MonsterPart<CNormalEnemy_Body>(
		CNormalEnemy_Body::PROTOTYPE_TAG, TEXT("Body"));
	if (nullptr == m_pBody)
		return E_FAIL;

	return S_OK;
}

void CNormalEnemy::On_Deserialized()
{
	__super::On_Deserialized();
}

CNormalEnemy* CNormalEnemy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNormalEnemy* pInstance = new CNormalEnemy(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CNormalEnemy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNormalEnemy::Clone(void* pArg)
{
	CNormalEnemy* pInstance = new CNormalEnemy(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNormalEnemy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNormalEnemy::Free()
{
	__super::Free();
}
