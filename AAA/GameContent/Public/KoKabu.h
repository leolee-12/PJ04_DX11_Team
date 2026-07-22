#pragma once
#include "PhysicsProjectile.h"
#include "Inhalable.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKokabu final : public CPhysicsProjectile, public IInhalable
{
	GENERATED_BODY(CKokabu)

private:
	enum class KOKABU_STATE
	{
		IDLE, EJECTED, FALLING, LANDED,
		HIT, DEATH, CAPTURED, SPAT, DISAPPEARING, END
	};

public:
	static constexpr const _tchar* PROTOTYPE_TAG	= L"Proto_Kokabu";
	static constexpr const _tchar* MODEL_PROTO_TAG	= L"Prototype_Component_Model_KoKabu";


private:
	CKokabu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKokabu(const CKokabu& Prototype);
	virtual ~CKokabu() = default;

public:
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

	// Inhalable
	virtual _bool				Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void				Be_Captured(CGameObject* pInhaler) override;
	virtual COPY_ABILITY_TYPE	Get_CopyAbility() const override
	{
		return COPY_ABILITY_TYPE::NONE;
	}
	virtual CGameObject*		Get_GameObject() override { return this; }
	virtual _float3				Get_SpatPivotOffset() const override
	{
		return m_vSpatPivot;
	}
	virtual void				On_SpatBegin() override;
	virtual void				On_SpatEnd()   override;

	void						On_Swallowed();


protected:
	virtual HRESULT				Initialize(void* pArg) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual HRESULT				Render() override;
	virtual HRESULT				Render_Shadow() override;
	virtual HRESULT				Ready_Visual() override;

	virtual	void				On_Launched() override;
	virtual void				On_Impact() override;

	virtual HRESULT				Ready_HitBox() override;

private:
	// »óÅÂ °ü·Ã
	void						Change_State(KOKABU_STATE eNext);
	void						Enter_State(KOKABU_STATE eState);
	void						Update_State(_float fTimeDelta);
	void						Exit_State(KOKABU_STATE eState);

	void						Update_Captured(_float fTimeDelta);
	void						Update_Spin(_float fTimeDelta);
	void						Update_SpatPivot_FromBone();
	void						Despawn();

	HRESULT						Bind_ShaderResources();

private:
	CShader*					m_pShaderCom = { nullptr };
	CModel*						m_pModelCom = { nullptr };
	CAnimator*					m_pAnimatorCom = { nullptr };

	KOKABU_STATE				m_eState = { KOKABU_STATE::IDLE };
	CGameObject*				m_pCaptor = { nullptr };
	_float3						m_vHitFrom = { 0.f, 0.f, 0.f };

	_float3                     m_vSpatPivot = { 0.f, 0.f, 0.f };
	_float						m_fSpinAngle = { 0.f };
	_float						m_fStateTimer = { 0.f };

	_float						m_fPullSpeed = { 0.f };
	_float3						m_vBaseScale = {};
	_float						m_fScaleRatio = { 1.f };

	static constexpr _float     s_fSpinSpeedDeg = { 720.f };
	static constexpr _float     s_fHitReactTime = { 0.6f };
	static constexpr _float     s_fDisappearTime = { 0.5f };
	static constexpr _float		s_fHitKnockback = { 8.f };
	static constexpr _float		s_fHitKnockUp = { 4.f };
	static constexpr _float		s_fPullAccel = { 40.f };
	static constexpr _float		s_fMinScaleRatio = { 0.45f };		// º¸¸é¼­ Æ©´×
	static constexpr _float		s_fShrinkLerp = { 2.f };

public:
	static CKokabu*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CKokabu*			Clone(void* pArg) override;

protected:
	virtual void				Free() override;

};

NS_END