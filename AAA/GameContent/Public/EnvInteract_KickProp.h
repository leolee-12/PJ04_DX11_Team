#pragma once
#include "EnvObject_Interact.h"
#include "Inhalable.h"

NS_BEGIN(Engine)
class CCollider;
class CRigidBody;
NS_END

NS_BEGIN(Client)

class CEnvInteract_KickProp final
	: public CEnvObject_Interact
	, public IInhalable
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

public: // Inhalable
	virtual _bool Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void Be_Captured(CGameObject* pInhaler) override;
	virtual COPY_ABILITY_TYPE Get_CopyAbility() const override { return COPY_ABILITY_TYPE::NONE; }
	virtual CGameObject* Get_GameObject() override { return this; }
	virtual void On_SpatBegin() override;
	virtual void On_SpatEnd() override;

private:
	CRigidBody* m_pRigidBodyCom = { nullptr };
	CCollider* m_pInteractCollider = { nullptr };
	_bool m_bKicked = { false };
	_float m_fPreviousVerticalVelocity = { 0.f };
	_float m_fKickedElapsed = { 0.f };
	_uint m_iBounceCount = { 0 };
	_bool m_bKickPending = { false };
	_float3 m_vPendingKickVelocity = {};

	CGameObject* m_pCaptor = { nullptr };
	_bool m_bCaptured = { false };
	_bool m_bSpat = { false };
	_float m_fPullSpeed = { 0.f };

	static constexpr _uint  s_iDisappearBounceCount = { 3 };     // Å± ¡æ 1È¸ ¹Ù¿î½º ¡æ ´ÙÀ½ ÂøÁö¿¡ ¼Ò¸ê
	static constexpr _float s_fKickYawDegree = { 8.f };          // Å± ¼ø°£ ÁÂ¿ì Èçµé¸²
	static constexpr _float s_fBounceYawDegree = { 30.f };       // Æ¥ ¶§ ¹æÇâ Æ²±â
	static constexpr _float s_fGravity = { -45.f };              // Ä¿ºñ/ÆøÅº°ú °°Àº Ã¼°¨ Áß·Â
	static constexpr _float s_fPhysXSceneGravity = { -9.81f };
	static constexpr _float s_fMaxKickLifeSecond = { 5.f };      // ³¶¶°·¯Áö·Î ³¯¾Æ°¡ ¿µ¿µ ¾È Æ¢´Â °æ¿ì º¸Çè
	static constexpr _float s_fPullAccel = { 40.f };

protected:
	virtual HRESULT Ready_InteractComponents() override;

private:
	HRESULT Ready_DynamicActor();
	HRESULT Ready_InteractCollider();
	void Sync_InteractCollider();
	void Handle_InteractColliderEnter(CCollider* pOther);
	void Kick_FromPlayer(CGameObject* pPlayer);
	void Update_Captured(_float fTimeDelta);
	void Update_BounceState(_float fTimeDelta);
	void Deactivate();
	void Clamp_DynamicVelocity();

public:
	static CEnvInteract_KickProp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END