#pragma once
#include "GameContent_Defines.h"
#include "Character.h"
#include "Monster_BlackBoard.h"

NS_BEGIN(physx)
class PxController;
NS_END

NS_BEGIN(Client)
class CMonster_Movement;
class CMonsterBrain;
class CMonster_StateMachine;

class CMonster abstract : public CCharacter
{
	GENERATED_BODY_ABSTRACT(CMonster)
	
	PROPERTY(_float, m_fHP, L"HP", L"Monster")
	PROPERTY(_float, m_fMaxHP, L"Max HP", L"Monster")
		
protected:
	CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster(const CMonster& Prototype);
	virtual ~CMonster() = default;

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

public:
	// AI가 이동 의도를 쌓는 방식
	void						Add_MoveDir(const _float3& vWishDir);
	_bool						Has_MoveDir() const;
	void						Clear_MoveDir();

	// Brain이 실행 FSM에 상태 전환을 요청
	void						Change_State(MONSTER_STATE_TYPE eNewState);
	MONSTER_STATE_TYPE			Get_StateType() const;

public:
	// 자식 몬스터가 자기 충돌 크기를 제공
	virtual _float				Get_CapsuleRadius() const = 0;
	virtual _float				Get_CapsuleHeight() const = 0;

	// 공통 State가 구체 몬스터 애니메이션을 호출하는 추상 훅
	virtual void				Play_StateAnimation(MONSTER_STATE_TYPE eState) = 0;

protected:
	physx::PxController*		m_pController = { nullptr };
	CMonster_Movement*			m_pMovement = { nullptr };

	MONSTER_BLACKBOARD			m_BlackBoard = {};

	CMonsterBrain*				m_pBrain = { nullptr };
	CMonster_StateMachine*		m_pStateMachine = { nullptr };

	// 이번 프레임에 이동하고 싶은 방향
	_float3						m_vWishDir = {};

protected:
	// 부모가 관리할 공통 파이프라인
	HRESULT						Ready_Movement();
	HRESULT						Ready_AI();

	// 윤석현 추가 AI 드라이버 선택 훅 (자식이 오버라이드)
	virtual CMonsterBrain*		Create_Brain(); //기본: FSM Brain
	virtual _bool				Use_StateMachine() const { return true; } // BT 전용 몬스터는 false 반환

	//윤석현 수정 
	virtual void				Update_AI(_float fTimeDelta);
	void						Perceive(_float fTimeDelta);

protected:
	virtual void				Free() override;
};

NS_END
