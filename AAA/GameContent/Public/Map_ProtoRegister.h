#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Client)

class CMap_ProtoRegister final : public CBase
{
private:
	CMap_ProtoRegister(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMap_ProtoRegister() = default;

public:
	HRESULT Ready_Prototypes(const MAP_RUNTIME_LEVELS& Levels, const MAP_PACKAGE& Package);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance_Proxy* m_pProxy = { nullptr };

private:
	HRESULT Ready_ObjectPrototypes(_uint iObjectLevel);
	HRESULT Ready_MapSectionModel(_uint iModelLevel, const MAP_SECTION_DESC& Desc);
	HRESULT Ready_EnvModel(_uint iModelLevel, const ENV_OBJECT_DESC& Desc);

	const _tchar* Get_EnvObjectProtoTag(ENV_OBJECT_KIND eKind) const;

public:
	static CMap_ProtoRegister* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	virtual void Free() override;
};

NS_END