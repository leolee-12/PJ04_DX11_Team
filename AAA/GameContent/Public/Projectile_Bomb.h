#pragma once
#include "PhysicsProjectile.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CProjectile_Bomb abstract : public CPhysicsProjectile
{
	GENERATED_BODY(CProjectile_Bomb)

protected:
	CProjectile_Bomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CProjectile_Bomb(const CProjectile_Bomb& Prototype);
	virtual ~CProjectile_Bomb() = default;

public:
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Update(_float fTimeDelta) override;		// 기존 수명 방식이 아닌 애니메이션 종료 여부로 수명 주기 따져야 해서 override 
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual HRESULT			Render_Shadow() override;

	virtual void			Despawn();
	void					Bomb_Explode();

protected:
	virtual HRESULT			Ready_Visual() override;
	virtual void			Tick_Visual(_float fTimeDelta) override;

	virtual void			On_Explode() {}
	virtual void			On_Impact() override;

	virtual HRESULT			Ready_AnimEvents() { return S_OK; }

private:
	HRESULT					Bind_ShaderResources();
	void					Roll_ByMovement(_float fTimeDelta);

protected:
	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	CAnimator*				m_pAnimatorCom = { nullptr };

protected:
	_float					m_fRollAngle = { 0.f };
	_float3					m_vRollAxis = { 1.f, 0.f, 0.f };
	static constexpr _float ROLL_DEG_PER_SPEED = { 45.f };

	_float					m_fBurnRatio = { 0.f };

	_float3					m_vGlow = { 0.f, 0.f, 0.f };

protected:
	virtual void			Free() override;
};

NS_END