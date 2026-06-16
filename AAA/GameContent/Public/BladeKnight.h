#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Client)

class CBladeKnight_Body;
class CBladeKnight_Sword;

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

private:
	HRESULT						Ready_PartObjects();
	HRESULT						Bind_ShaderResources();

	virtual void				On_Deserialized() override;

private:
	CBladeKnight_Body*			m_pBody = { nullptr };
	CBladeKnight_Sword*			m_pSword = { nullptr };


public:
	static CBladeKnight*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext); 
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;
};

NS_END
