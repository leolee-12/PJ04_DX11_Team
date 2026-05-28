#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;

class ENGINE_DLL CComponent abstract : public CBase
{
protected:
	CComponent(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CComponent(const CComponent& Prototype);
	virtual ~CComponent() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render() { return S_OK; }

protected:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };

	_bool m_isCloned = { false };


public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END