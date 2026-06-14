#pragma once

#include "EnvObject.h"

NS_BEGIN(Client)
class CEnv_InstanceController;

class CLIENT_DLL CEnvObject_Static final : public CEnvObject
{
public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvObject_Static";

private:
	CEnvObject_Static(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject_Static(const CEnvObject_Static& Prototype);
	virtual ~CEnvObject_Static() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render_Shadow() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:	// Instance
	void	Set_InstanceController(CEnv_InstanceController* pCtrl);
	_bool	Can_RenderInstance() const;

private:
	CEnv_InstanceController* m_pInstanceController = { nullptr };
	ENV_INSTANCE_BATCH_HANDLE m_InstanceBatchHandle = {};

private:
	void Submit_RenderGroups();

public:
	static CEnvObject_Static* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END