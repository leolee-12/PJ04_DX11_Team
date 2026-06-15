#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CMap_Spawner final : public CBase
{
private:
	CMap_Spawner();
	virtual ~CMap_Spawner() = default;

public:
	HRESULT Spawn(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_RESULT* pOutReport = nullptr);

private:
	CGameInstance_Proxy* m_pProxy = { nullptr };

private:
	const MAP_SPAWN_ROUTE* Resolve_EnvRoute(const MAP_SPAWN_TARGETS& Targets, ENV_OBJECT_KIND eKind) const;
	const _tchar* Get_EnvObjectProtoTag(ENV_OBJECT_KIND eKind) const;
	_wstring Make_EnvObjectName(const ENV_OBJECT_DESC& Desc) const;
	void Rollback(const vector<CGameObject*>& CreatedObjects);

public:
	static CMap_Spawner* Create();

private:
	virtual void Free() override;
};

NS_END