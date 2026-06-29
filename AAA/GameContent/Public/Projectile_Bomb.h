#pragma once
#include "PhysicsProjectile.h"
#include "Inhalable.h"

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
	virtual HRESULT			Render() override;

protected:
	virtual HRESULT			Ready_Visual() override;
	virtual void			Tick_Visual(_float fTimeDelta) override;

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
	static constexpr _float ROLL_DEG_PER_SPEED = { 90.f };

protected:
	virtual void			Free() override;
};

NS_END