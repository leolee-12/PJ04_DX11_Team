#pragma once

#include "EnvObject.h"

NS_BEGIN(Client)

class CLIENT_DLL CEnvObject_Static final : public CEnvObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_EnvObject_Static";

private:
	CEnvObject_Static(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject_Static(const CEnvObject_Static& Prototype);
	virtual ~CEnvObject_Static() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	static CEnvObject_Static* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void Free() override;
};

NS_END
