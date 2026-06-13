#pragma once
#include "GameObject.h"
#include "GameContent_Defines.h"
#include "EnvObject_Defines.h"

NS_BEGIN(Client)
class CEnvObject_Static;
class CEnv_InstanceBatch;

class CEnv_InstanceController final : public CGameObject
{
public:
	static constexpr const _tchar* PROTOTYPE_TAG = TEXT("Proto_Env_InstanceController");

private:
	CEnv_InstanceController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnv_InstanceController(const CEnv_InstanceController& Prototype);
	virtual ~CEnv_InstanceController() = default;

public:
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }
	
	_bool Submit_Main(CEnvObject_Static* pObj);
	_bool Submit_Shadow(CEnvObject_Static* pObj);

private:
	unordered_map<ENV_INSTANCE_KEY, CEnv_InstanceBatch*, ENV_INSTANCE_KEY_HASH> m_Batches;

private:
	CEnv_InstanceBatch* FindOrCreate_Batch(ENV_INSTANCE_KEY tKey, CEnvObject_Static* pObj);

public:
	static CEnv_InstanceController* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END