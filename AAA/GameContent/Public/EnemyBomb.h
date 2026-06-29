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
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual _bool				Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void				Be_Captured(CGameObject* pInhaler) override;
	virtual void				Be_Spat(_fvector vPos, _fvector vDir, _float fSpeed) override;

	virtual COPY_ABILITY_TYPE	Get_CopyAbility() const override
	{
		return COPY_ABILITY_TYPE::BOMB;
	}

	virtual CGameObject*		Get_GameObject() override { return this; }

	void						On_Swallowed();


protected:
	virtual void				Update(_float fTimeDelta) override;
	virtual HRESULT				Ready_Visual() override;

	virtual void				On_Activated() override;

	virtual void				On_Bounce(_int iCount) override;

	virtual void				On_Explode() override;

private:
	_bool						m_bCaptured = { false };
	CGameObject*				m_pCaptor = { nullptr };

public:
	static CEnemyBomb*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;
};

NS_END