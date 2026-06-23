#pragma once
#include "Base.h"
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CLevelDesign_Spawner final : public CBase
{
private:
	CLevelDesign_Spawner();
	virtual ~CLevelDesign_Spawner() = default;

public:
	HRESULT Spawn(const LD_PACKAGE& Package, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pOutReport = nullptr);

private:
	CGameInstance_Proxy* m_pProxy = { nullptr };

private:
	HRESULT Spawn_One(const LD_OBJECT_ENTRY& Desc, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pInOutReport, CGameObject** ppOutCreatedObject);
	_wstring Make_ObjectTag(const LD_OBJECT_DESC& Desc) const;

public:
	static CLevelDesign_Spawner* Create();

private:
	virtual void Free() override;
};

NS_END