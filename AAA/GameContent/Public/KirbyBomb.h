#pragma once
#include "Projectile_Bomb.h"

NS_BEGIN(Engine)
class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CKirbyBomb final : public CProjectile_Bomb
{
	GENERATED_BODY(CKirbyBomb)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_KirbyBomb";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_KirbyBomb";
	static constexpr const _tchar* POOL_KEY = L"KirbyBomb";

private:
	CKirbyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirbyBomb(const CKirbyBomb& Prototype);
	virtual ~CKirbyBomb() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

protected:
	virtual void	Update(_float fTimeDelta) override;
	virtual HRESULT	Ready_AnimEvents() override;
	virtual HRESULT Ready_HitBox() override;

	virtual HRESULT	Ready_Visual() override;

	virtual void	On_Activated() override;
	virtual void	On_Bounce(_int iCount) override;
	virtual void	On_Explode() override;

public:
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }
	virtual void Despawn() override;

private:
	void Update_FuseSocket();
	void Stop_Fuse();

private:
	CEffect_Container*  m_pFuseFx{};
	const _float4x4*	m_pFuseBone{};
	_float4x4			m_matFuseWorld{};

public:
	static CKirbyBomb* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END
