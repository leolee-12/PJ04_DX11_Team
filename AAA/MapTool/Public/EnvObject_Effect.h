#pragma once

#include "EnvObject.h"

NS_BEGIN(Client)

class CEnvObject_Effect final : public CEnvObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_EnvObject_Effect";

private:
	CEnvObject_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject_Effect(const CEnvObject_Effect& Prototype);
	virtual ~CEnvObject_Effect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	static CEnvObject_Effect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void Free() override;
};

NS_END
