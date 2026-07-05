#pragma once
#include "LevelDesignObject.h"
#include "Inhalable.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLD_Inhalable abstract : public CLevelDesignObject, public IInhalable
{
protected:
	CLD_Inhalable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_Inhalable(const CLevelDesignObject& Prototype);
	virtual ~CLD_Inhalable() = default;

	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT	Validate_Initialized() override;

public:
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;

public:
	virtual _bool Can_BeInhaled(const INHALE_QUERY& q) const override { return true; }
	virtual void Be_Captured(CGameObject* pInhaler) override;
	virtual COPY_ABILITY_TYPE Get_CopyAbility() const override { return COPY_ABILITY_TYPE::NONE; }
	virtual CGameObject* Get_GameObject() override { return this; }
	virtual void On_SpatBegin() override;
	virtual void On_SpatEnd()   override;

protected:
	enum class LD_STATE { IDLE, CAPTURED };

	LD_STATE m_eState = { LD_STATE::IDLE };

	CCollider* m_pHurtBox = { nullptr };

	_float	   m_fColliderRadius = { 1.f };

	CGameObject* m_pCaptor = { nullptr };

	_float						m_fPullSpeed = { 0.f };
	_float3						m_vBaseScale = {};
	_float						m_fScaleRatio = { 1.f };

	_float                      m_fSpinAngle = { 0.f };
	_float                      m_fLifeTime = { 0.f };

	_float					    s_fSpatDamage = { 50.f };
	_float					    s_fSpatKnockback = { 10.f };

	static constexpr _float		s_fPullInitSpeed = 0.f;
	static constexpr _float		s_fPullAccel = 40.f;
	static constexpr _float		s_fMinScaleRatio = 0.15f;
	static constexpr _float		s_fShrinkLerp = 2.f;

	static constexpr _float     s_fMaxLifeTime = 2.5f;
	static constexpr _float     s_fSpinSpeedDeg = 360.f;

protected:
	HRESULT			Ready_Collider();
	virtual void	SetUp_Collider_CallBack();

	virtual void	On_Swallowed();
	virtual void	Enable_Colliders(_bool b);
	virtual void	Despawn_Spat();

private:
	void Update_Captured(_float fTimeDelta);
};

NS_END