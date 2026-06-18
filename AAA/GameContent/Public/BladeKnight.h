#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Client)

class CBladeKnight_Body;
class CBladeKnight_Sword;
class CMonsterBrain;

class CBladeKnight final : public CMonster
{
	GENERATED_BODY(CBladeKnight)

public:
	enum class BLADEKNIGHT_AI_STYLE
	{
		STATIONARY,		// 발견하면 이동 안함
		CHASE			// 발견하면 추격
	};

	enum class BLADEKNIGHT_ATTACK_TYPE
	{
		ATTACK,
		DOUBLE_ATTACK,
		TORNADO_ATTACK
	};

	struct BLADEKNIGHT_ATTACK_PATTERN
	{
		BLADEKNIGHT_ATTACK_TYPE eType = { BLADEKNIGHT_ATTACK_TYPE::ATTACK };
		MONSTER_STATE_TYPE		eNextState = { MONSTER_STATE_TYPE::RETREAT };
	};

public:
	struct BLADEKNIGHT_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_BladeKnight";

private:
	CBladeKnight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBladeKnight(const CBladeKnight& Prototype);
	virtual ~CBladeKnight() = default;

public:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	virtual void				Priority_Update(_float fTimeDelta) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Late_Update(_float fTimeDelta) override;
	virtual HRESULT				Render() override;

	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}
	
	virtual _float				Get_CapsuleRadius() const override;
	virtual _float				Get_CapsuleHeight() const override;
	virtual void				Play_StateAnimation(MONSTER_STATE_TYPE eState) override;

	virtual _bool				Is_StateAnimationFinished() const override;

public:
	CBladeKnight_Body*			Get_Body() { return m_pBody; }
	CBladeKnight_Sword*			Get_Sword() { return m_pSword; }

	_bool						Try_SelectAttackPattern(_float fDistXZ);
	BLADEKNIGHT_ATTACK_TYPE		Get_CurrentAttackType() const { return m_tCurAttackPattern.eType; }
	MONSTER_STATE_TYPE			Get_AttackNewState() const { return m_tCurAttackPattern.eNextState; }

	BLADEKNIGHT_AI_STYLE		Get_AIStyle() const { return m_eAIStyle; }
	void Set_AIStyle(BLADEKNIGHT_AI_STYLE eStyle) { m_eAIStyle = eStyle; }

protected:
	virtual CMonsterBrain*		Create_Brain() override;
	virtual HRESULT				Ready_State(CMonster_StateMachine* pStateMachine) override;
	virtual HRESULT				Ready_AnimEvents() override;

private:
	HRESULT						Ready_PartObjects();
	HRESULT						Bind_ShaderResources();

	virtual void				On_Deserialized() override;

private:
	CBladeKnight_Body*			m_pBody = { nullptr };
	CBladeKnight_Sword*			m_pSword = { nullptr };

	// 테스트용 멤버변수
	_float						m_fTiltCurDeg = { 0.f };		// 현재 누적 기울기 
	_float						m_fTiltLerp = { 5.0f };			// 클수록 빨리 도달한다.

	BLADEKNIGHT_AI_STYLE		m_eAIStyle = { BLADEKNIGHT_AI_STYLE::CHASE };
	BLADEKNIGHT_ATTACK_PATTERN	m_tCurAttackPattern{};

	_uint						m_iAttackPatternIndex = { 0 };

public:
	static CBladeKnight*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext); 
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;
};

NS_END
