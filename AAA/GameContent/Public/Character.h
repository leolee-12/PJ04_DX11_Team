#pragma once

#include "GameContent_Defines.h"
#include "ContainerObject.h"
#include "Damageable.h"

NS_BEGIN(Client)

class CCharacter abstract : public CContainerObject, public IDamageable
{
	GENERATED_BODY_ABSTRACT(CContainerObject)

protected:
	CCharacter(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CCharacter(const CCharacter& Prototype);
	virtual ~CCharacter() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual void Damaged(const ATTACK_INFO& tInfo) override;

	const _float* Get_HitFlashPtr()      const { return &m_fHitFlashCur; }
	const _float3* Get_HitFlashColorPtr() const { return &m_vHitFlashColor; }

	void    Start_Invincibility() { Start_Invincibility(m_fInvincibleDuration); }
	void    Start_Invincibility(_float fDuration);
	_bool   Is_Invincible() const { return m_fInvincibleTime > 0.f; }

protected:
	// 히트스톱: 파생 클래스는 Update 첫 줄에서 dt를 이걸로 걸러 쓴다.
	  // (Priority_Update에는 적용 금지 - 베이스 타이머가 실시간으로 돌아야 함)
	_float  Filter_TimeDelta(_float fTimeDelta) const { return (m_fHitStopTime > 0.f) ? 0.f : fTimeDelta; }
	_bool   Is_HitStopped() const { return m_fHitStopTime > 0.f; }

	// 파생 훅 
	virtual void On_InvincibleBegin() {}    
	virtual void On_InvincibleEnd() {}

protected:
	_float						m_fMaxHP = { 100.f };
	_float						m_fCurHP = { 100.f };

	_float  m_fHitFlashCur = { 0.f };          // 현재 강도 0..1
	_float  m_fHitFlashTime = { 0.f };          // 원샷 잔여시간(몬스터)
	_float  m_fHitFlashDuration = { 0.12f };        // 원샷 1회 길이
	_float3 m_vHitFlashColor = { 1.f, 1.f, 1.f };// 흰색

	// 히트 리액션 (스톱 + 렌더 셰이크)
	_float  m_fHitStopTime = { 0.f };                       // 남은 정지 시간
	_float  m_fHitStopDuration = { 0.2f };          // 피격 1회당 정지 시간
	_float  m_fShakeTime = { 0.f };                         // 남은 셰이크 시간
	_float  m_fShakeDuration = { 0.2f };
	_float  m_fShakeAmp = { 0.1f };                        // 진폭(월드 단위). 취향껏 조정
	_float  m_fShakeElapsed = { 0.f };                      // 셰이크 경과(사인 위상용)

	_float  m_fInvincibleTime = { 0.f };
	_float  m_fInvincibleDuration = { 0.2f };

protected:
	virtual _bool Block_Hit(const ATTACK_INFO& tInfo) { return false; }
	virtual void  On_Damaged(const ATTACK_INFO& tInfo) {}
	virtual void  On_Death(const ATTACK_INFO& tInfo) {}

public:
	virtual CGameObject* Clone(void* pArg) = 0;
protected:
	virtual void Free() override;
};

NS_END