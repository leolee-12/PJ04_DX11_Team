#pragma once
#include "EnvObject_Trigger.h"

NS_BEGIN(Client)

class CEnvVolume_Culling final : public CEnvObject_Trigger
{
	GENERATED_BODY(CEnvVolume_Culling)

	PROPERTY(_wstring, m_strHideKind, L"Hide Kind", L"Culling Volume")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvVolume_Culling";

private:
	CEnvVolume_Culling(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvVolume_Culling(const CEnvVolume_Culling& Prototype);
	virtual ~CEnvVolume_Culling() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;

#ifdef _DEBUG
protected:
	virtual _wstring Get_DebugLabel() const override;
#endif

public:
	static CEnvVolume_Culling* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END