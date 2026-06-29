#pragma once
#include "Projectile_Bomb.h"

NS_BEGIN(Client)

class CEnemyBomb final : public CProjectile_Bomb
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

protected:
	virtual HRESULT			Ready_Visual() override;
	virtual void			Update_Terminal(_float dt) override;
	virtual void			Tick_Visual(_float dt) override;

	virtual void			On_Bounce(_int iCount) override;	 // 첫 바운스 시
	virtual void			On_Activated() override;		//	부착 되었을 때

	virtual void			On_Impact() override;		// 충돌 시 



public:
	static CEnemyBomb*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void			Free() override;
};

NS_END