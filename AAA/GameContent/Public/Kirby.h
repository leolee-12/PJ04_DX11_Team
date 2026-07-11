#pragma once

#include "Character.h"

#include "GameContent_const.h"
#include "Kirby_Command.h"

NS_BEGIN(Engine)
class CMovement;
class CCollider;
class CController;
NS_END

NS_BEGIN(Client)

enum class KIRBY_STATE_TYPE;
enum class COPY_ABILITY_TYPE;

class CKirby_InputManager;
class CKirby_Controller;
class CKirby_StateMachine;

class CMovement_Child;

class CKirby_AttackMode;
class CKirby_Ability;
class CKirby_Deform;

class CKirby_Body;
class CKirby_OnOffPart;
class CKirby_Deform_Model;

class CLevelDesign_Ladder;

class IDeformable;

enum class KIRBY_DEFORM_MODEL_TYPE { DEMO, MAIN };

class CKirby final : public CCharacter
{
	GENERATED_BODY(CKirby)

public:
	enum KIRBY_COLLIDER { HURT_BOX, INHALE_BOX ,COLLIDER_END };

	struct KIRBY_BODY_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby";

	// Controller(Collider: Capsule)
	static constexpr _float s_fCCT_Radius = 0.5f;
	static constexpr _float s_fCCT_Height = 0.1f;
	static constexpr _float s_fHurtBoxRadiusPadding = 0.1f;

	// Movement_Child
	static constexpr _float s_fGroundFriction = 40.f;
	static constexpr _float s_fMaxHorizontalSpeed = 9.f;

	static constexpr _float s_fLinearDrag = 0.9f;
	static constexpr _float s_fFallVelocityY = -7.f;
	static constexpr _float s_fMaxFallVelocity = -15.f;

	static constexpr _float s_fRot_Speed_Degree = 720.f;

	// À±¼®Çö Ãß°¡ 
	static constexpr _float s_fInhaleFwd = 1.8f;
	static constexpr _float s_fInhaleUp = 0.5f;
	static constexpr _float s_fInhaleRadius = 3.f;
	static constexpr _float s_fInhaleLength = 2.f;

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
	// Ability AnimEvent
	_bool Dispatch_BodyAnimEventToAbility(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase);

	// Com
	CMovement_Child* Get_Movement() { return m_pMovement; }

	// Part
	void Set_AbilityPartsActive(COPY_ABILITY_TYPE eAbilityType, _bool bOn, _bool bOnlyWeapon = false);
	void Change_HatSocketMatrix(COPY_ABILITY_TYPE eAbilityType, const _float4x4* pBoneMatrix);

	CKirby_OnOffPart* Find_OnOffPart(const wchar_t* PartTag);
	CKirby_OnOffPart* Find_WeaponPart(COPY_ABILITY_TYPE eType);
	CKirby_OnOffPart* Find_HatPart(COPY_ABILITY_TYPE eType);

	void Put_WeaponOnBack(_bool bOn);

	CKirby_Body* Get_Body() { return m_pBody; }
	CKirby_Deform_Model* Get_DeformPart_Model(DEFORM_TYPE eDeformType, KIRBY_DEFORM_MODEL_TYPE eDeformModelType = KIRBY_DEFORM_MODEL_TYPE::MAIN);

	CKirby_Deform_Model* Find_DeformModel(const wchar_t* pPartTag);

	// Controller
	void Set_CollisionSize(_float fCCTRadius, _float fCCTHeight);

	// Movement
	void Add_MoveDir(const _float3& vWishDir);
	_bool Has_MoveDir();
	void Set_RotationLock(_bool RotationLock) { m_RotationLock = RotationLock; }

	//System
	void Excute_Command(CKirby_Command* pCommand);
	void Change_State(KIRBY_STATE_TYPE eNewState, _int iFlag = -1);

	// Attack Mode
	CKirby_AttackMode* Get_ActiveAttackMode();

	// Model
	CKirby_Deform_Model* Get_CurrentDeformModel();

	// Ability
	CKirby_Ability* Get_KirbyAbility();
	void Request_ChangeKirbyAbility(COPY_ABILITY_TYPE eAbilityState);
	void Apply_ChangeKirbyAbility();

	// Ability Dump
	void Update_DumpCool(_float fTimeDelta);
	void Reset_DumpCool();
	_bool Can_Dump();
	void Req_AbilityDumpCoolDecrease() { m_bDecreaseDumpCool = true; }

	// Deform
	_bool Has_Deform() { return m_pKirby_Deform ? true : false; }
	CKirby_Deform* Get_KirbyDeform() { return m_pKirby_Deform; }
	void Change_KirbyDeform(DEFORM_TYPE eDeformType);
	void Reset_KirbyDeform();

	// Collider
	CCollider* Get_Collider(KIRBY_COLLIDER eKirbyCollider);

	// CutScene Attach
	void Update_CutsceneAttachTransform();

	// Damage
	virtual void Damaged(const ATTACK_INFO& tInfo) override;
	void Add_HP(_float fHP);
	void Start_DamageInvincibility();

	// Ladder
	CLevelDesign_Ladder* Get_Ladder() { return m_pLadder; }
	void Set_Ladder(CLevelDesign_Ladder* pLadder) { m_pLadder = pLadder; }
	void Clear_Ladder() { m_pLadder = nullptr; };
	_bool IsLadder() { return m_pLadder ? true : false; }

	//Deform Object
	IDeformable* Get_TriggerDeformObj() { return m_pTriggerDeformObj; }
	void Set_TriggerDeformObj(IDeformable* pTriggerDeformObj) { m_pTriggerDeformObj = pTriggerDeformObj; }
	_bool IsTriggerDeformObj() { return m_pTriggerDeformObj != nullptr; }

	IDeformable* Get_HeldDeformObj() { return m_pHeldDeformObj; }
	void Set_HeldDeformObj(IDeformable* pHeldDeformObj) { m_pHeldDeformObj = pHeldDeformObj; }

private:
	HRESULT Ready_Components();
	HRESULT	SetUp_Collider_Callback();
	HRESULT Ready_PartObjects();
	HRESULT Ready_System();
	HRESULT Ready_Ability();
	HRESULT Ready_Deform();
	HRESULT Bind_ShaderResources();
	virtual HRESULT Ready_Events() override;
	HRESULT Ready_AnimEvents();

	// Ability
	void Set_KirbyAbility(COPY_ABILITY_TYPE eAbilityState);

	// Damage
	virtual _bool Block_Hit(const ATTACK_INFO& tInfo) override;
	virtual void  On_Damaged(const ATTACK_INFO& tInfo) override;

	// CutScene Attach
	void Set_CutsceneAttachTarget(const KIRBY_ATTACHMENT_BEGIN_DESC* pAttachDesc);
	void Clear_CutsceneAttachTarget();

	// Level SPawn
	void Warp_To(const _float3& vPosition, const _float3& vLook);
	void Republish_HUDState();

private:
	// Parts
	CKirby_Body* m_pBody{};

	// Com
	CController* m_pController{};
	CMovement_Child* m_pMovement{};

	// Movement
	_float3 m_vWishDir{};
	_bool m_RotationLock{};

	// Collider
	vector<CCollider*> m_KirbyColliders;

	void Update_BlobShadow();
	void Update_InvincibilityHitFlash();

private:
	// System
	CKirby_InputManager*	m_pKirby_InputManager{};
	CKirby_Controller*		m_pKirby_Controller{};
	CKirby_StateMachine*	m_pKirby_StateMachine{};

	// Ability
	CKirby_Ability*			m_pKirby_Ability{};
	unordered_map<COPY_ABILITY_TYPE, CKirby_Ability*> m_Abilities;

	_bool m_bReqChangeAbility{};
	COPY_ABILITY_TYPE m_eNextAbilityType{};

	// Ability Dump
	_float m_fAccDumpCoolTime{};
	_float m_fMaxDumpCoolTime{ 0.5f };
	_bool m_bDecreaseDumpCool{};

	// Deform
	CKirby_Deform* m_pKirby_Deform{};
	unordered_map<DEFORM_TYPE, CKirby_Deform*> m_Deformations;

	// CutScene Attach
	_float3 m_vPreAttachScale{};
	const _float4x4* m_pAttachBone{};
	const _float4x4* m_pAttachOwnerWorld{};

	// Ladder
	CLevelDesign_Ladder* m_pLadder{};

	//Deform Object
	IDeformable* m_pTriggerDeformObj{};
	IDeformable* m_pHeldDeformObj{};

	// Level Spawn
	const void* m_pLastSpawner = { nullptr };

public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END