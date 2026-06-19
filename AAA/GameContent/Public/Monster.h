#pragma once
#include "GameContent_Defines.h"
#include "Character.h"
#include "Monster_BlackBoard.h"
#include "Inhalable.h"

NS_BEGIN(Engine)
class CCollider;
class CController;
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

public:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

	virtual void				Priority_Update(_float fTimeDelta) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Late_Update(_float fTimeDelta) override;
	virtual HRESULT				Render() override;

	virtual void				On_Deserialized() override;

public: 
	// 외부에서 타겟 주입
	void						Set_Target(CGameObject* pTarget);

	// Brain/State가 상황 데이터를 볼 수 있는 통로
	const MONSTER_BLACKBOARD&	Get_BlackBoard() const { return m_BlackBoard; }
	MONSTER_BLACKBOARD&			Get_BlackBoard() { return m_BlackBoard; }
	CMonster_Movement*			Get_Movement() { return m_pMovement; }
	_bool						Has_Trait(MONSTER_TRAIT t) const { return (m_TraitFlags & t) != 0; }
	COPY_ABILITY_TYPE		    Get_CopyAbility() const { return m_eCopyAbility; }

public: // Inhalable
	virtual _bool				Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void				Be_Captured(CGameObject* pInhaler) override;

	CGameObject*				Get_Captor() const { return m_pCaptor; }
	void						On_Swallowed();

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

public:
	// 자식 몬스터가 자기 충돌 크기를 제공
	virtual _float				Get_CapsuleRadius() const = 0;
	virtual _float				Get_CapsuleHeight() const = 0;
	virtual _float				Get_InteractRadius() const = 0;
	virtual _float				Get_HurtBoxRadius() const = 0;

	// 공통 State가 구체 몬스터 애니메이션을 호출하는 추상 훅
	virtual void				Play_StateAnimation(MONSTER_STATE_TYPE eState) = 0;

	// 애니메이션이 끝났는지 노출해주는 함수
	virtual _bool				Is_StateAnimationFinished() const { return true; }

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

	// 이번 프레임에 이동하고 싶은 방향
	_float3						m_vWishDir = {};

	_uint					    m_TraitFlags = { MT_DEFAULT };
	COPY_ABILITY_TYPE			m_eCopyAbility = { COPY_ABILITY_TYPE::NORMAL };
	CGameObject*			    m_pCaptor = { nullptr };

protected:
	// 부모가 관리할 공통 파이프라인
	HRESULT						Ready_Collider();
	void						SetUp_Collider_CallBack();
	HRESULT						Ready_Movement();
	HRESULT						Ready_AI();

	// 윤석현 추가 AI 드라이버 선택 훅 (자식이 오버라이드)
	virtual CMonsterBrain*		Create_Brain();		//기본: FSM Brain
	virtual HRESULT				Create_Movement();	// 기본 : Monster_Movement - 별도 무브먼트 필요할 때 상속받아서 쓸 것 
	virtual _bool				Use_StateMachine() const { return true; } // BT 전용 몬스터는 false 반환
	virtual HRESULT				Ready_State(CMonster_StateMachine* pStateMachine);
	virtual HRESULT				Ready_AnimEvents() { return S_OK; }		// Bkody의 Animator의 이벤트 콜백 설정함수

	virtual void				On_Damaged(const ATTACK_INFO& tInfo) override;
	virtual void				On_Death(const ATTACK_INFO& tInfo) override;

	//윤석현 수정 
	virtual void				Update_AI(_float fTimeDelta);
	virtual void				Perceive(_float fTimeDelta);

protected:
	virtual void				Free() override;
};

NS_END
