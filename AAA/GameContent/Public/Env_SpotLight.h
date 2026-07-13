#pragma once
#include "EnvObject.h"

NS_BEGIN(Client)

class CEnv_SpotLight final : public CEnvObject
{
	GENERATED_BODY(CEnv_SpotLight)

	PROPERTY(_float3, m_vDirection, L"Direction", L"Spot Light")
		PROPERTY(_float4, m_vColor, L"Color", L"Spot Light")
		PROPERTY(_float, m_fIntensity, L"Intensity", L"Spot Light")
		PROPERTY(_float, m_fRange, L"Range", L"Spot Light")
		PROPERTY(_float, m_fAngle, L"Angle", L"Spot Light")
		PROPERTY(_float, m_fDecayStartAngle, L"Decay Start Angle", L"Spot Light")
		PROPERTY(_wstring, m_strKind, L"Kind", L"Spot Light")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Env_SpotLight";

private:
	CEnv_SpotLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnv_SpotLight(const CEnv_SpotLight& Prototype);
	virtual ~CEnv_SpotLight() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	static CEnv_SpotLight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END