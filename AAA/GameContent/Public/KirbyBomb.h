#pragma once
#include "Projectile_Bomb.h"

NS_BEGIN(Client)

class CKirbyBomb final : public CProjectile_Bomb
{
	GENERATED_BODY(CKirbyBomb)

private:
	enum class BOMB_STATE
	{
		NONE,
		HELD,			// 몬스터 손에 부착(심지 이동 정지)
		FLYING,			// 투척(심지 점화)
		DANGER,			// 첫 바운드 후 (붉은색 발광 효과)
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

protected:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	Update(_float fTimeDelta) override;
	virtual HRESULT Ready_Visual() override;
	virtual HRESULT Ready_AnimEvents() override;
	virtual HRESULT Ready_HitBox() override;

	// 활성화/발사/바운드 
	virtual void On_Activated() override;
	virtual void On_Launched() override;
	virtual void On_Bounce(_int iCount) override;

private:
	void Change_State(BOMB_STATE eNext);
	void Enter_State(BOMB_STATE eState);
	void Update_State(_float fTimeDelta);
	void Exit_State(BOMB_STATE eState);

private:
	BOMB_STATE m_eState = { BOMB_STATE::NONE };

public:
	static CKirbyBomb* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END