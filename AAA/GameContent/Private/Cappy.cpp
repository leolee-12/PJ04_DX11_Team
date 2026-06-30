#include "Cappy.h"
#include "GameInstance.h"

#include "Cappy_Body.h"
#include "Cappy_Brain.h"

#include "Monster_StateMachine.h"
#include "Monster_State_Idle.h"
#include "Monster_Movement.h"

CCappy::CCappy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CCappy::CCappy(const CCappy& Prototype)
	: CMonster ( Prototype )
{
}

HRESULT CCappy::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CCappy::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eCopyAbility = COPY_ABILITY_TYPE::NONE;

	if (m_pTransformCom)
		m_pTransformCom->Set_RotationPerSec(180.f);

	return S_OK;
}

void CCappy::Priority_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CCappy::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

#ifdef _DEBUG
	if (m_pGameInstance_Proxy->Is_EditMode())
	{
		if (m_pMovement) m_pMovement->Sync_To_Controller();
		return;
	}
#endif

	__super::Update(fTimeDelta);
}

void CCappy::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);
}

_bool CCappy::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
	Out.fRadius = { 0.5f };
	Out.fHeight = { 0.75f };

	return true;
}

CAnimator* CCappy::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CCappy::Create_Brain()
{
	return CCappy_Brain::Create(this);
}

HRESULT CCappy::Ready_State(CMonster_StateMachine* pStateMachine)
{
	if (nullptr == m_pStateMachine)
		return E_FAIL;

	ANI_PLAY_INFO Info{};
	
	// 임시 작성 
	// IDLE
	Info.strAniName = "Wait";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
		return E_FAIL;

	return S_OK;
}

HRESULT CCappy::Ready_AnimEvents()
{
	return S_OK;
}

HRESULT CCappy::Ready_PartObjects()
{
	m_pBody = Add_MonsterPart<CCappy_Body>(CCappy_Body::PROTOTYPE_TAG, TEXT("Body"));
	if (nullptr == m_pBody)
		return E_FAIL;

	return S_OK;
}

void CCappy::On_Deserialized()
{
	__super::On_Deserialized();
}

void CCappy::On_Swallowed()
{
	__super::On_Swallowed();
}

CCappy* CCappy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCappy* pInstance = new CCappy(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCappy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCappy::Clone(void* pArg)
{
	CCappy* pInstance = new CCappy(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCappy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappy::Free()
{
	__super::Free();
}
