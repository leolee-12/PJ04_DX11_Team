#pragma once

#include "GameContent_Defines.h"
#include "Movement.h"

NS_BEGIN(Client)

class CMonster_Movement : public CMovement
{
	GENERATED_BODY(CMonster_Movement)

protected:
	CMonster_Movement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster_Movement(const CMonster_Movement& Prototype);
	virtual ~CMonster_Movement() = default;

public:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	void						Set_LockFacing(_bool bLock) { m_bLockFacing = bLock; }
	void						Face_Smooth(_fvector vWorldTarget, _float fTimeDelta);
	void						Face_Instant(_fvector vWorldTarget);

	void						Set_WindowMoveSpeed(_float fPeak, _float p01);

	// 발사 함수 (넉백 시 사용)
	void						Launch(_fvector vHorizDir, _float fHorizSpeed, _float fUpSpeed);
	_bool						Is_Launched() const { return m_bLaunched; }
	_bool						Update_Launched(_float fTimeDelta);
	// 윤석현 추가
	void						Knockback(_fvector vAttackerPos, _float fStrength);
	void						KO(_fvector vAttackerPos, _float fStrength);
	void						Begin_JumpArc(_fvector vTargetPos, _float fDuration, _float fHeight);
	_bool						Is_JumpArc() const { return m_bJumpArc; }
	_bool						Update_JumpArc(_float fTimeDelta);

	_bool						Is_KO()       const { return m_bKO; }
	_bool						Is_Bouncing() const { return m_bBouncing; }

public:
	static CMonster_Movement*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*			Clone(void* pArg) override;

protected:
	virtual void				Apply_Facing(_fvector vFaceDir, _float fTimeDelta) override;

private:
	_bool						m_bLockFacing = { false };
	_bool						m_bLaunched = { false };
	_bool						m_bKO = { false };
	_bool						m_bBouncing = { false };
	_int						m_iBounceCount = { 0 };
	_int						m_iMaxBounce = { 1 };

	// 바운스 계수
	_float						m_fRestitution = { 0.6f };			// 수직 반발 계수 (0~1) : 클수록 높이 튄다
	_float						m_fBounceFriction = { 0.2f };		// 튕길 때마다 수평 감쇠(0~1)
	_float						m_fBounceStopSpeed = { 1.0f };		// 반사속도가 이보다 작으면 정착

	// 시간기반포물선점프
	_bool						m_bJumpArc = { false };
	_float3						m_vJumpStart = {};
	_float3						m_vJumpTarget = {};
	_float						m_fJumpDur = { 0.f };
	_float						m_fJumpHeight = { 0.f };
	_float						m_fJumpT = { 0.f };

private:
	void						Start_Launch(_fvector vAttackerPos, _float fStrength, _float fUpRatio);

protected:
	virtual void				Free() override;
};

NS_END