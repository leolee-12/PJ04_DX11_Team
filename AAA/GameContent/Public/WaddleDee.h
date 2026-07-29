#pragma once
#include "Character.h"

NS_BEGIN(Engine)
class CCollider;
class CController;
class CMovement;
NS_END

NS_BEGIN(Client)
class CWaddleDee_Body;
class CWaddleDee_Hat;

class CWaddleDee final : public CCharacter
{
	GENERATED_BODY(CWaddleDee)

	PROPERTY(_wstring, m_strFixedAnim, L"FixedAnim", L"NPC")
	PROPERTY(_bool, m_bWander, L"Wander", L"NPC")
	PROPERTY(_float, m_fWanderRadius, L"WanderRadius", L"NPC")
	PROPERTY(_float, m_fWalkSpeed, L"WalkSpeed", L"NPC")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_WaddleDee";

	enum class WADDLEDEE_STATE : _uint
	{
		IDLE,
		WALK,
		GREET,
		HIT,
		ANGRY
	};

	enum class WADDLEDEE_EMOTE : _uint
	{
		WAVE,
		YAY,
		_COUNT
	};

private:
	CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaddleDee(const CWaddleDee& Prototype);
	virtual ~CWaddleDee() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Damaged(const ATTACK_INFO& tInfo) override;
	virtual void Set_Active(_bool bActive) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

	void React_Emote(WADDLEDEE_EMOTE eEmote);

private:
	CWaddleDee_Body* m_pBody = { nullptr };
	CWaddleDee_Hat* m_pHat = { nullptr };
	CCollider* m_pHurtBox = { nullptr };
	CController* m_pController = { nullptr };
	CMovement* m_pMovement = { nullptr };

	WADDLEDEE_STATE m_eState = { WADDLEDEE_STATE::IDLE };
	_float m_fStateTimer = { 0.f };
	_string m_strInteractClip = {};
	_wstring m_strObjectName = {};
	_bool m_bUseMovement = { true };
	_wstring m_strAppliedFixedAnim = {};
	_float m_fGreetCooldown = { 0.f };

	_bool m_bBasePosCaptured = { false };
	_float3 m_vBasePos = {};
	_float3 m_vWalkTarget = {};
	_float3 m_vLookOrigin = {};
	_float3 m_vLookTarget = {};

	CGameObject* m_pPlayer = { nullptr };

private:
	virtual void On_Deserialized() override;

	HRESULT Ready_PartObjects();
	HRESULT Ready_Movement();
	HRESULT Ready_AnimEvents();
	HRESULT Ready_HurtBox();
	HRESULT Validate_Initialized();
	void Sync_MovementStats();
	_bool Is_Sitting() const;

	void Change_State(WADDLEDEE_STATE eState);
	_bool Find_Player();

	void Update_Idle(_float fTimeDelta);
	void Update_Walk(_float fTimeDelta);
	void Play_Idle();
	void Pick_WalkTarget();
	void Update_Greet(_float fTimeDelta);
	void Update_Hit(_float fTimeDelta);
	void Update_Angry(_float fTimeDelta);

public:
	static _wstring Resolve_FixedAnim(const _wstring& strVariation, _uint iSeed = 0u);
	static CWaddleDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END