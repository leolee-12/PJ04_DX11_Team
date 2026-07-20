#pragma once
#include "EnvObject_Interact.h"

NS_BEGIN(Engine)
class CCollider;
class CRigidBody;
NS_END

NS_BEGIN(Client)

class CEnvInteract_KickProp final : public CEnvObject_Interact
{
public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvInteract_KickProp";

private:
	CEnvInteract_KickProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvInteract_KickProp(const CEnvInteract_KickProp& Prototype);
	virtual ~CEnvInteract_KickProp() = default;

private:
	virtual HRESULT Initialize_Prototype() override;

public:
	virtual void Late_Update(_float fTimeDelta) override;

protected:
	virtual HRESULT Ready_InteractComponents() override;

private:
	HRESULT Ready_DynamicActor();
	HRESULT Ready_InteractCollider();
	void Sync_InteractCollider();
	void Handle_InteractColliderEnter(CCollider* pOther);
	void Kick_FromPlayer(CGameObject* pPlayer);
	void Update_BounceState();
	void Deactivate();
	void Clamp_DynamicVelocity();

private:
	CRigidBody* m_pRigidBodyCom = { nullptr };
	CCollider* m_pInteractCollider = { nullptr };
	_bool m_bKicked = { false };
	_float m_fPreviousVerticalVelocity = { 0.f };
	_uint m_iBounceCount = { 0 };
	_bool m_bKickPending = { false };
	_float3 m_vPendingKickVelocity = {};

	static constexpr _uint s_iDisappearBounceCount = { 3 };

public:
	static CEnvInteract_KickProp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END