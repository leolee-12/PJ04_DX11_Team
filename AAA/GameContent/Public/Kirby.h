#pragma once

#include "Character.h"

#include "GameContent_const.h"
#include "Kirby_Command.h"

NS_BEGIN(physx)
class PxController;
NS_END

NS_BEGIN(Engine)
class CMovement;
NS_END

NS_BEGIN(Client)

class CMovement_Child;

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

	// Controller(Collider: Capsule)
	static constexpr _float s_fCCT_Radius = 0.75f;
	static constexpr _float s_fCCT_Height = 0.2f;

	static constexpr _float s_fFallVelocityY = -7.f;

	static constexpr _float s_fLinearDrag = 0.9f;
	static constexpr _float s_fHoveringLinearDrag = 9.f;

	static constexpr _float s_fMaxFallVelocity = -15.f;
	static constexpr _float s_fHoveringMaxFallVelocity = -1.5f;

	static constexpr _float s_fMaxHorizontalSpeed = 8.f;
	static constexpr _float s_fHoveringMaxHorizontalSpeed = 4.f;

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

	virtual void On_Deserialized() override;

public:
	// Com
	CMovement_Child* Get_Movement() { return m_pMovement; }

	// Part
	CKirby_Body* Get_Body() { return m_pBody; }
	void OnOffParts(KIRBY_ABILITY_TYPE eAbilityType, _bool fOn);

public:
	// Movement
	void Add_MoveDir(const _float3& vWishDir);
	_bool Has_MoveDir();

public:
	//System
	void Excute_Command(CKirby_Command* pCommand);
	void Change_State(KIRBY_STATE_TYPE eNewState);

	CKirby_Ability* Get_KirbyAbility();
	void Set_KirbyAbility(KIRBY_ABILITY_TYPE eAbilityState);

public:
	// Ability Dump
	void Update_AbilityDumpCool(_float fTimeDelta);
	void Reset_AbilityDumpCool();
	_bool Can_AbilityDump();
	void Req_AbilityDumpCoolDecrease() { m_bDecreaseAbilityDumpCool = true; }

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();
	HRESULT Ready_System();
	HRESULT Ready_Ability();
	HRESULT Bind_ShaderResources();

private:
	CKirby_Body* m_pBody{};

	physx::PxController* m_pController{};
	CMovement_Child* m_pMovement{};

	_float3 m_vWishDir{};

private:
	CKirby_InputManager*	m_pKirby_InputManager{};
	CKirby_Controller*		m_pKirby_Controller{};
	CKirby_StateMachine*	m_pKirby_StateMachine{};
	CKirby_Ability*			m_pKirby_Ability{};

private:
	_float m_fAccAbilityDumpCoolTime{};
	_float m_fMaxAbilityDumpCoolTime{ 0.5f };
	_bool m_bDecreaseAbilityDumpCool{};

public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END