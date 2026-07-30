#pragma once
#include "EnvObject_Interact.h"
#include "Damageable.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CEnvInteract_BreakProp final
	: public CEnvObject_Interact
	, public IDamageable
{
public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvInteract_BreakProp";

private:
	CEnvInteract_BreakProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvInteract_BreakProp(const CEnvInteract_BreakProp& Prototype);
	virtual ~CEnvInteract_BreakProp() = default;

private:
	virtual HRESULT Initialize_Prototype() override;

public:
	virtual void Late_Update(_float fTimeDelta) override;

public: // Damageable
	virtual void Damaged(const ATTACK_INFO& tInfo) override;

protected:
	virtual HRESULT Ready_InteractComponents() override;

private:
	HRESULT Ready_RigidStatic();
	HRESULT Ready_HurtBox();
	void Grant_Reward();

private:
	enum class BREAK_STATE
	{
		INTACT,
		DESTROYED
	};

	CCollider* m_pHurtBox = { nullptr };
	BREAK_STATE m_eState = { BREAK_STATE::INTACT };

public:
	static CEnvInteract_BreakProp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END