#pragma once
#include "Monster.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)
class CPoppyBrosJr_Body;
class CMonsterBrain;
class CMonster_StateMachine;
class CProjectile;

class CPoppyBrosJr final : public CMonster
{
	GENERATED_BODY(CPoppyBrosJr)

public:
	struct POPPYBROSJR_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_PoppyBrosJr";

private:
	CPoppyBrosJr(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPoppyBrosJr(const CPoppyBrosJr& Prototype);
	virtual ~CPoppyBrosJr() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize(void* pArg) override;

public:
	virtual void		Priority_Update(_float fTimeDelta) override;
	virtual void		Update(_float fTimeDelta) override;
	virtual void		Late_Update(_float fTimeDelta) override;

	virtual void		Copy_PrototypeName(
		ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual _float Get_CapsuleRadius() const override { return 0.5f; }
	virtual _float Get_CapsuleHeight() const override { return 0.75f; }
	virtual _float Get_InteractRadius() const override
	{
		return (m_iAIType == 0) ? 15.f : 10.f;
	}
	virtual _bool			Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

	virtual CAnimator*		Get_BodyAnimator() const override;

	CPoppyBrosJr_Body*		Get_Body() { return m_pBody; }

	_int					Get_AIType() const { return m_iAIType; }
	void					Set_AIType(_int iType) { m_iAIType = iType; }

protected:
	virtual CMonsterBrain*	Create_Brain() override;
	virtual HRESULT			Ready_State(CMonster_StateMachine* pStateMachine) override;
	virtual HRESULT			Ready_PartObjects() override;
	virtual HRESULT			Ready_AnimEvents() override;


private:
	CPoppyBrosJr_Body*		m_pBody = { nullptr };
	_int					m_iAIType = { 1 };		// 0: 이동 - 고정, 공격 고정 방향 , 1 : 이동 고정 + 플레이어 방향으로 공격 , 2 : 이동 자유(Patrol + Chase) - 공격 플레이어 방향

	CProjectile*			m_pHeldBomb = { nullptr };

private:
	void					Attach_Bomb();
	void					Throw_Bomb();

public:
	static CPoppyBrosJr*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void			Free() override;
};

NS_END