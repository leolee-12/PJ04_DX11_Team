#pragma once
#include "Projectile_Bomb.h"
#include "Inhalable.h"

NS_BEGIN(Client)

class CEnemyBomb final : public CProjectile_Bomb, public IInhalable
{
	GENERATED_BODY(CEnemyBomb)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnemyBomb";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_EnemyBomb";

private:
	CEnemyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnemyBomb(const CEnemyBomb& Prototype);
	virtual ~CEnemyBomb() = default;

public:
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	// IInhalable을(를) 통해 상속됨
	_bool Can_BeInhaled(const INHALE_QUERY& q) const override { return true; }
	void Be_Captured(CGameObject* pInhaler) override;
	COPY_ABILITY_TYPE Get_CopyAbility() const override { return COPY_ABILITY_TYPE::BOMB; }
	void Be_Spat(_fvector vPos, _fvector vDir, _float fSpeed) override {}
	CGameObject* Get_GameObject() override { return this; }

protected:
	virtual HRESULT			Ready_Visual() override;

	virtual void			On_Activated() override;

	virtual void			On_Bounce(_int iCount) override;

	virtual void			On_Explode() override;


public:
	static CEnemyBomb*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void			Free() override;
};

NS_END