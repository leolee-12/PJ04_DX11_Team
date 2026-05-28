#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;

class CShadow_Dir final : public CBase
{
private:
	CShadow_Dir();
	virtual ~CShadow_Dir() = default;

public:
	const _float4x4* Get_Transform(D3DTS eState) const {
		return &m_TransformStateMatrices[ETOUI(eState)];
	}

public:
	HRESULT Add_ShadowLight(const SHADOW_LIGHT_DESC& ShadowDesc);
	HRESULT Update_ShadowLight(const SHADOW_LIGHT_DESC& Desc) {
		return Add_ShadowLight(Desc);
	}

private:
	CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };
	_float4x4			 m_TransformStateMatrices[ETOUI(D3DTS::END)] = {};

public:
	static CShadow_Dir* Create();
	virtual void Free() override;
};

NS_END