#pragma once
#include "GameContent_Defines.h"
#include "Character.h"
#include "Monster_BlackBoard.h"
#include "Inhalable.h"
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CCollider;
class CController;
class CAnimator;
NS_END

NS_BEGIN(Client)
class CMonster_Movement;
class CMonsterBrain;
class CMonster_StateMachine;

class CMonster abstract : public CCharacter, public IInhalable
{
	GENERATED_BODY_ABSTRACT(CMonster)

protected:
	CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster(const CMonster& Prototype);
	virtual ~CMonster() = default;

public:
	enum MONSTER_TRAIT : _uint {
		MT_NONE				= 0,
		MT_INHALABLE		= 1 << 0,   
		MT_BODYCHECK_DAMAGE = 1 << 1,
		MT_STRONG_INHALE_ONLY = 1 << 2,

		MT_DEFAULT = MT_INHALABLE | MT_BODYCHECK_DAMAGE,
	};

	struct HIT_REACTION		// 상태에 전달할 피격 정보 
	{
		_float3					vAttackerPos = {};
		_float					fKnockBack = 0.f;
		_float					fDamage = 0.f;
	};

public:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

	virtual void				Priority_Update(_float fTimeDelta) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Late_Update(_float fTimeDelta) override;

	virtual void				On_Deserialized() override;

public: 
	// 외부에서 타겟 주입
	void						Set_Target(CGameObject* pTarget);

	// Brain/State가 상황 데이터를 볼 수 있는 통로
	const MONSTER_BLACKBOARD&	Get_BlackBoard() const { return m_BlackBoard; }
	MONSTER_BLACKBOARD&			Get_BlackBoard() { return m_BlackBoard; }
	CMonster_Movement*			Get_Movement() { return m_pMovement; }
	const _float3&				Get_BasePos() const { return m_vBasePos; }
	_bool						Has_Trait(MONSTER_TRAIT t) const { return (m_TraitFlags & t) != 0; }

	_int						Get_AIType() const { return m_iAIType; }

public: // Inhalable
	virtual _bool				Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void				Be_Captured(CGameObject* pInhaler) override;
	virtual void				Be_Spat(_fvector vPos, _fvector vDir, _float fSpeed) override;
	virtual COPY_ABILITY_TYPE	Get_CopyAbility() const override { return m_eCopyAbility; }
	virtual CGameObject*		Get_GameObject() override final { return this; }

	virtual void				On_Swallowed();
	virtual void				On_Exit(MONSTER_STATE_TYPE eNextState) {}
	const _float3&				Get_SpatVelocity() const { return m_vSpatVelocity; }
	void						Enable_ProjectileBox(_bool bEnable);
	CGameObject*				Get_Captor() const { return m_pCaptor; }
	void						Despawn_Spat();                                
	void						Despawn();

public:
	// AI가 이동 의도를 쌓는 방식
	void						Add_MoveDir(const _float3& vWishDir);
	void						Add_MoveDir(_fvector vWishDir);
	_bool						Has_MoveDir() const;
	void						Clear_MoveDir();

	// Brain이 실행 FSM에 상태 전환을 요청
	_bool						Change_State(MONSTER_STATE_TYPE eNewState);
	_bool						Has_State(MONSTER_STATE_TYPE eState) const;
	MONSTER_STATE_TYPE			Get_StateType() const;

	HIT_REACTION				Get_LastHit() { return m_LastHit; }

public:
	// 자식 몬스터가 자기 충돌 크기를 제공
	virtual _float				Get_CapsuleRadius() const = 0;
	virtual _float				Get_CapsuleHeight() const = 0;
	virtual _float				Get_InteractRadius() const = 0;
	virtual _bool				Get_HurtBoxDesc(CAPSULE_DESC& Out) const = 0;

	virtual CAnimator*			Get_BodyAnimator() const { return nullptr; }

	virtual _bool				Is_Touch_Harmful() const { return true; }

	// 윤석현 추가
	void						Enable_Controller(_bool bEnable);
	void						Enable_Colliders(_bool bEnable);

protected:
	CController*				m_pController = { nullptr };
	CMonster_Movement*			m_pMovement = { nullptr };

	MONSTER_BLACKBOARD			m_BlackBoard = {};

	CMonsterBrain*				m_pBrain = { nullptr };
	CMonster_StateMachine*		m_pStateMachine = { nullptr };

	CCollider*					m_pInteractCollider = { nullptr };
	CCollider*					m_pHurtBox = { nullptr };
	CCollider*					m_pProjectileBox = { nullptr };
	_float3						m_vSpatVelocity = {};

	// 이번 프레임에 이동하고 싶은 방향
	_float3						m_vWishDir = {};

	_uint					    m_TraitFlags = { MT_DEFAULT };
	COPY_ABILITY_TYPE			m_eCopyAbility = { COPY_ABILITY_TYPE::NONE };
	CGameObject*			    m_pCaptor = { nullptr };

	static constexpr _float		s_fSpatDamage = 100.f;
	static constexpr _float		s_fSpatKnockback = 12.f;

	HIT_REACTION				m_LastHit{};		// 상태에 던질 값들 

	_float3						m_vBasePos = {};

	_float						m_fAirborneTimer = 0.f;
	static constexpr _float		s_fCoyoteTime = 0.12f;

	_int						m_iAIType = { 0 };

protected:
	// 부모가 관리할 공통 파이프라인
	HRESULT						Ready_Collider();
	void						SetUp_Collider_CallBack();
	HRESULT						Ready_Movement();
	HRESULT						Ready_AI();
	void						Check_AirborneReflex(_float fTimeDelta);

	virtual CMonsterBrain*		Create_Brain();		//기본: FSM Brain
	virtual HRESULT				Create_Movement();	// 기본 : Monster_Movement - 별도 무브먼트 필요할 때 상속받아서 쓸 것 
	virtual _bool				Use_StateMachine() const { return true; } // BT 전용 몬스터는 false 반환
	virtual HRESULT				Ready_State();
	virtual HRESULT				Ready_PartObjects() { return S_OK; }
	virtual HRESULT				Ready_AnimEvents() { return S_OK; }		// Bkody의 Animator의 이벤트 콜백 설정함수

	virtual void				On_Damaged(const ATTACK_INFO& tInfo) override;
	virtual void				On_Death(const ATTACK_INFO& tInfo) override;

	virtual _bool				Block_Hit(const ATTACK_INFO& tInfo) override;

	//윤석현 수정 
	virtual void				Update_AI(_float fTimeDelta);
	virtual void				Perceive(_float fTimeDelta);

	virtual void				Apply_AIVariation(const _wstring& strVariation) {}

	_bool						Handle_SharedAnimEvent(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase);

protected:
	template<class TPart>
	TPart* Add_MonsterPart(const _tchar* szProtoTag, const _tchar* szPartTag, const _float4x4* pSocketBone = nullptr)
	{
		CMonsterPart::MONSTERPART_DESC Desc{};
		Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		Desc.pSocketBoneMatrix = pSocketBone;

		if (FAILED(Add_PartObject(m_iPrototypeLevel, szProtoTag, szPartTag, &Desc)))
			return nullptr;

		return dynamic_cast<TPart*>(m_PartObjects[szPartTag]);
	}

protected:
	virtual void				Free() override;
};

NS_END
