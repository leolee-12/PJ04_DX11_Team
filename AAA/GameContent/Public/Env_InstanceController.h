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
	ENV_INSTANCE_BATCH_HANDLE Register_BatchesForDesc(const ENV_OBJECT_DESC& tDesc);
	_bool Submit_Main(_uint iBatchIndex, CEnvObject_Static* pObj);
	_bool Submit_Shadow(_uint iBatchIndex, CEnvObject_Static* pObj);

private:
	vector<CEnv_InstanceBatch*> m_Batches;
	unordered_map<ENV_INSTANCE_KEY, _uint, ENV_INSTANCE_KEY_HASH> m_BatchIndexByKey;

private:
	_uint FindOrCreate_BatchIndex(const ENV_INSTANCE_KEY& tKey);

public:
	static CEnv_InstanceController* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END