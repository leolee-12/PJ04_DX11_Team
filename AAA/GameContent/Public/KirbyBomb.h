#pragma once
#include "Projectile_Bomb.h"

NS_BEGIN(Client)

class CMovement_Child;

class CKirbyBomb final : public CProjectile_Bomb
{
	GENERATED_BODY(CKirbyBomb)

private:
	enum class KIRBYBOMB_STATE
	{
		NONE,
		HELD,
		THROW,
		DANGER,
		EXPLODEPRE
	};

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_KirbyBomb";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_KribyBomb";
	static constexpr const _tchar* POOL_KEY = L"KirbyBomb";

private:
	CKirbyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirbyBomb(const CKirbyBomb& Prototype);
	virtual ~CKirbyBomb() = default;

public:
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }
	virtual void Launch(const _float3& vPos, const _float3& vDir) override;

	void Launch(const _float3& vPos, const _float3& vDir, _float fLaunchSpeed);

	_float3 Cal_LaunchVelocity(const _float3& vHorizontalDir, _float fHorizontalSpeed, _float fArcHeight);
	void Launch_Velocity(const _float3& vStart, const _float3& vVelocity);

protected:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Tick_Visual(_float fTimeDelta) override;

	virtual HRESULT Ready_Movement() override;
	virtual HRESULT Ready_Visual() override;
	virtual HRESULT Ready_HitBox() override;
	virtual void Kill() override;

	// 활성화/발사/바운드
	virtual void On_Activated() override;
	virtual void On_Launched() override;
	virtual void On_Bounce(_int iCount) override;

private:
	void Change_State(KIRBYBOMB_STATE eNext);
	void Enter_State(KIRBYBOMB_STATE eState);
	void Update_State(_float fTimeDelta);
	void Exit_State(KIRBYBOMB_STATE eState);
	void Update_BombMovement(_float fTimeDelta);
	void Roll_ByBombMovement(_float fTimeDelta);

private:
	CMovement_Child* m_pBombMovement = { nullptr };

	KIRBYBOMB_STATE m_eState = { KIRBYBOMB_STATE::NONE };

	static constexpr _int s_iMaxExplodeAniPlayCount = { 5 };
	_int m_iExplodeAniPlayCount{};

public:
	static CKirbyBomb* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END
