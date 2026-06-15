#pragma once
#include "GameContent_Defines.h"
#include "Character.h"

NS_BEGIN(Client)

class CBladeKnight_Body;

class CBladeKnight final : public CCharacter
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

	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}
	
public:
	CBladeKnight_Body*			Get_Body() { return m_pBody; }

private:
	HRESULT						Ready_Components();
	HRESULT						Ready_PartObjects();
	HRESULT						Bind_ShaderResources();

	virtual void				On_Deserialized() override;

private:
	CBladeKnight_Body*			m_pBody = { nullptr };


public:
	static CBladeKnight*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext); 
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;
};

NS_END
