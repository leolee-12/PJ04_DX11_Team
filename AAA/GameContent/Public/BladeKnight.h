#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CBladeKnight_Body;
class CBladeKnight_Sword;
class CMonsterBrain;

class CBladeKnight final : public CMonster
{
	GENERATED_BODY(CBladeKnight)

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

	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}
	
	virtual _float				Get_CapsuleRadius() const override { return 0.5f; }
	virtual _float				Get_CapsuleHeight() const override { return 1.f; }
	virtual _float				Get_InteractRadius() const override { return 10.f; }
	virtual _bool				Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

	virtual CAnimator*			Get_BodyAnimator() const override;

public:
	CBladeKnight_Body*			Get_Body() { return m_pBody; }
	CBladeKnight_Sword*			Get_Sword() { return m_pSword; }

	// BladeKnight 고정형/자유 이동형 설정
	_int						Get_AIType() { return m_iAIType; }
	void						Set_AIType(_int iType) { m_iAIType = iType; }

protected:
	virtual CMonsterBrain*		Create_Brain() override;
	virtual HRESULT				Ready_State(CMonster_StateMachine* pStateMachine) override;
	virtual HRESULT				Ready_AnimEvents() override;

private:
	virtual HRESULT				Ready_PartObjects() override;

	virtual void				On_Deserialized() override;

private:
	CBladeKnight_Body*			m_pBody = { nullptr };
	CBladeKnight_Sword*			m_pSword = { nullptr };

	_int						m_iAIType = { 0 };		// 0은 고정형, 1은 자유 이동형

	_bool						m_bIdleOverlayOn = { false };

public:
	static CBladeKnight*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext); 
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;
};

NS_END
