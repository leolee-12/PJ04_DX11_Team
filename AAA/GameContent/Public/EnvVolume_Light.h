#pragma once
#include "EnvObject_Trigger.h"

NS_BEGIN(Client)

class CEnvVolume_Light final : public CEnvObject_Trigger
{
	GENERATED_BODY(CEnvVolume_Light)

	PROPERTY(_wstring, m_strAreaLightName, L"Area Light Name", L"Light Volume")
		PROPERTY(_float, m_fInTransitionSec, L"In Transition Sec", L"Light Volume")
		PROPERTY(_float, m_fOutTransitionSec, L"Out Transition Sec", L"Light Volume")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvVolume_Light";

private:
	CEnvVolume_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvVolume_Light(const CEnvVolume_Light& Prototype);
	virtual ~CEnvVolume_Light() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

#ifdef _DEBUG
protected:
	virtual _wstring Get_DebugLabel() const override;
#endif

public:
	static CEnvVolume_Light* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END