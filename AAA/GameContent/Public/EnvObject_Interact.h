#pragma once

#include "EnvObject.h"

NS_BEGIN(Client)

class CLIENT_DLL CEnvObject_Interact final : public CEnvObject
{
public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvObject_Interact";

private:
	CEnvObject_Interact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject_Interact(const CEnvObject_Interact& Prototype);
	virtual ~CEnvObject_Interact() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	static CEnvObject_Interact* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void Free() override;
};

NS_END
