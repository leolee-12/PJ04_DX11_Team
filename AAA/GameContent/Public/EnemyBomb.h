#pragma once
#include "Projectile_Bomb.h"
#include "Inhalable.h"

NS_BEGIN(Client)

class CEnemyBomb final : public CProjectile_Bomb, public IInhalable
{
	GENERATED_BODY(CEnemyBomb)

private:
	enum class BOMB_STATE
	{
		NONE, 
		HELD,			// 몬스터 손에 부착(심지 이동 정지)
		FLYING,			// 투척(심지 점화)
		DANGER,			// 첫 바운드 후 (붉은색 발광 효과)
		CAPTURED		// 커비 흡입
	};

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnemyBomb";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_EnemyBomb";

private:
	CEnemyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnemyBomb(const CEnemyBomb& Prototype);
	virtual ~CEnemyBomb() = default;

public:
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual _bool				Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void				Be_Captured(CGameObject* pInhaler) override;
	virtual void				On_SpatBegin() override {}
	virtual void				On_SpatEnd()   override {}

	virtual COPY_ABILITY_TYPE	Get_CopyAbility() const override
	{
		return COPY_ABILITY_TYPE::BOMB;		
	}

	virtual CGameObject*		Get_GameObject() override { return this; }

	void						On_Swallowed();


protected:
	virtual HRESULT				Initialize(void* pArg) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual HRESULT				Ready_Visual() override;
	virtual HRESULT				Ready_AnimEvents() override;

	// 활성화/발사/바운드 
	virtual void				On_Activated() override;
	virtual void				On_Launched() override;
	virtual void				On_Bounce(_int iCount) override;

private:
	// 상태 관련
	void						Change_State(BOMB_STATE eNext);
	void						Enter_State(BOMB_STATE eState);
	void						Update_State(_float fTimeDelta);
	void						Exit_State(BOMB_STATE eState);

	void						Update_Captured(_float fTimeDelta);	


private:
	BOMB_STATE					m_eState = { BOMB_STATE::NONE };

	CGameObject*				m_pCaptor = { nullptr };

	_float						m_fPullSpeed = { 0.f };
	_float3						m_vBaseScale = {};
	_float						m_fScaleRatio = { 1.f };

	static constexpr _float		s_fPullAccel = { 40.f };
	static constexpr _float		s_fMinScaleRatio = { 0.45f };		// 보면서 튜닝
	static constexpr _float		s_fShrinkLerp = { 2.f };

public:
	static CEnemyBomb*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;
};

NS_END