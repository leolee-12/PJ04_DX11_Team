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

	// 발사 함수 (넉백 시 사용)
	void						Launch(_fvector vHorizDir, _float fHorizSpeed, _float fUpSpeed);

	_bool						Is_Launched() const { return m_bLaunched; }
	_bool						Update_Launched(_float fTimeDelta);

public:
	static CMonster_Movement*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*			Clone(void* pArg) override;

protected:
	virtual void				Apply_Facing(_fvector vFaceDir, _float fTimeDelta) override;

private:
	_bool						m_bLockFacing = { false };
	_bool						m_bLaunched = { false };

	// 바운스 계수
	_float						m_fRestitution = { 0.5f };			// 수직 반발 계수 (0~1) : 클수록 높이 튄다
	_float						m_fBounceFriction = { 0.2f };		// 튕길 때마다 수평 감쇠(0~1)
	_float						m_fBounceStopSpeed = { 1.0f };		// 반사속도가 이보다 작으면 정착

protected:
	virtual void				Free() override;
};

NS_END