#pragma once

#include "Character.h"

#include "Kirby_Command.h"

NS_BEGIN(physx)
class PxController;
NS_END

NS_BEGIN(Engine)
class CMovement;
NS_END

NS_BEGIN(Client)

class CKirby_InputManager;
class CKirby_Controller;
class CKirby_StateMachine;
class CKirby_Ability;

enum class KIRBY_STATE_TYPE;
enum class KIRBY_ABILITY_TYPE;

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

public:
	CMovement* Get_Movement() { return m_pMovement; }
	CKirby_Body* Get_Body() { return m_pBody; }

public:
	void Add_MoveDir(const _float3& vWishDir);
	_bool Has_MoveDir();

public:
	void Excute_Command(CKirby_Command* pCommand);
	void Change_State(KIRBY_STATE_TYPE eNewState);

	CKirby_Ability* Get_KirbyAbility();
	void Set_KirbyAbility(CKirby_Ability* pKirby_Ability);

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();
	HRESULT Ready_System();
	HRESULT Ready_Ability();
	HRESULT Bind_ShaderResources();

	virtual void On_Deserialized() override;

private:
	CKirby_Body* m_pBody{};

	physx::PxController* m_pController = { nullptr };
	CMovement* m_pMovement = { nullptr }; 

	// Controller(Collider: Capsule)
	static constexpr _float CCT_RADIUS = 0.75f;
	static constexpr _float CCT_HEIGHT = 0.2f;

	// Movement
	static constexpr _float MOVE_SPEED = 7.0f;
	static constexpr _float ROT_SPEED = 720.0f;   // degree/sec
	static constexpr _float GRAVITY = -36.0f;
	static constexpr _float JUMP_SPEED = 17.0f;

	static constexpr _float MOVE_ACCEL = 130.f;
	static constexpr _float MOVE_DECEL = 70.f;

	_float3 m_vWishDir{};

private:
	CKirby_InputManager*	m_pKirby_InputManager{};
	CKirby_Controller*		m_pKirby_Controller{};
	CKirby_StateMachine*	m_pKirby_StateMachine{};
	CKirby_Ability*			m_pKirby_Ability{};

public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END