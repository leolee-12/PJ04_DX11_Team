#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)
class CCappy_Brain;
class CCappy_Body;
class CCappy_Hat;

class CCappy final : public CMonster
{
	GENERATED_BODY(CCappy)

	PROPERTY(_int, m_iAIType, L"AI Type", L"AI")	// 0은 고정형 / 1은 플레이어 추격형

public:
	struct CAPPY_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Cappy";

private:
	CCappy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCappy(const CCappy& Prototype);
	virtual ~CCappy() = default;

protected:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	virtual void				Priority_Update(_float fTimeDelta) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Late_Update(_float fTimeDelta) override;

	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual _float				Get_CapsuleRadius() const override { return 0.5f; }
	virtual _float				Get_CapsuleHeight() const override { return 0.75f; }
	virtual _float				Get_InteractRadius() const override { return (m_iAIType == 0) ? 15.f : 10.f; }
	virtual _bool				Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

	virtual CAnimator*			Get_BodyAnimator() const override;

public:
	CCappy_Body*				Get_Body() { return m_pBody; }
	CCappy_Hat*					Get_Hat() { return m_pHat; }

	_int						Get_AIType() { return m_iAIType; }

protected:
	virtual CMonsterBrain*		Create_Brain() override;
	virtual HRESULT				Ready_State(CMonster_StateMachine* pStateMachine) override;
	virtual HRESULT				Ready_AnimEvents() override;

private:
	virtual HRESULT				Ready_PartObjects() override;

	virtual void				On_Deserialized() override;

	virtual void				On_Swallowed() override;

private:
	CCappy_Body*				m_pBody = { nullptr };
	CCappy_Hat*					m_pHat = { nullptr };

public:
	static CCappy*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;


};

NS_END