#pragma once

#include "Character.h"

NS_BEGIN(physx)
class PxController;
NS_END

NS_BEGIN(Engine)
class CMovement;
NS_END

NS_BEGIN(Client)

class CKirby_Body;

class CKirby final : public CCharacter
{
	GENERATED_BODY(CCharacter)

public:
	struct KIRBY_BODY_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby";

private:
	CKirby(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CKirby(const CKirby& Prototype);
	virtual ~CKirby() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();
	HRESULT Bind_ShaderResources();

	virtual void On_Deserialized() override;

private:
	CKirby_Body* m_pBody{};

	physx::PxController* m_pController = { nullptr };
	CMovement* m_pMovement = { nullptr }; 

	//controller
	static constexpr _float CCT_RADIUS = 0.75f;
	static constexpr _float CCT_HEIGHT = 0.2f;

	//movement
	static constexpr _float MOVE_SPEED = 6.0f;
	static constexpr _float ROT_SPEED = 720.0f;   // degree/sec
	static constexpr _float GRAVITY = -20.0f;
	static constexpr _float JUMP_SPEED = 8.0f;
	static constexpr _float MOVE_ACCEL = 40.f;
	static constexpr _float MOVE_DECEL = 50.f;


public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END