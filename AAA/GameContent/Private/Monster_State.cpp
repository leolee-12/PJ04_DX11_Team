#include "Monster_State.h"

CMonster_State::CMonster_State()
	: m_bIsInterruptible{ false }
{
}

HRESULT	CMonster_State::Initialize()
{
	m_bIsInterruptible = false;

	return S_OK;
}

HRESULT	CMonster_State::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	m_PlayInfo = tInfo;
	m_fSpeed = fSpeed;

	m_bIsInterruptible = false;

	return S_OK;
}

void CMonster_State::Enter(CMonster* pMonster)
{

}

void CMonster_State::Update(CMonster* pMonster, _float fTimeDelta)
{
}

void CMonster_State::Exit(CMonster* pMonster)
{
}

void CMonster_State::Free()
{
	__super::Free();
}
