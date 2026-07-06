#include "Cappy_State_KasaUp.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CCappy_State_KasaUp::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = false;

	return S_OK;
}

MONSTER_STATE_TYPE CCappy_State_KasaUp::Get_StateType()
{
	return MONSTER_STATE_TYPE::JUMP;
}

void CCappy_State_KasaUp::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;

	Play_KasaUpAnimation();
}

void CCappy_State_KasaUp::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;
}

void CCappy_State_KasaUp::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CCappy_State_KasaUp* CCappy_State_KasaUp::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CCappy_State_KasaUp* pInstance = new CCappy_State_KasaUp();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CCappy_State_KasaUp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappy_State_KasaUp::Play_KasaUpAnimation()
{
	if (m_pOwner == nullptr || m_pAnimator == nullptr)
		return;

	_int iRand = rand() % 3;

	ANI_PLAY_INFO		Info{};
	LAYER_PLAY_INFO		LayerInfo{};

	switch (iRand)
	{
	case 0:		
	{
		Info.strAniName = "KasaUp1";
		break;
	}
	case 1:
	{
		Info.strAniName = "KasaUp2";
		break;
	}
	case 2:		
	{
		Info.strAniName = "KasaUp3";
		break;
	}
	default:	
	{
		break;
	}
	}
}

void CCappy_State_KasaUp::Free()
{
	__super::Free();
}
